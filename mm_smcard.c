/*
 * Code to dump Smartcard table from Nortel Millennium Payphone
 * Table 93 (0x5d)
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#ifdef _WIN32
# include <winsock.h>
#else  /* ifdef _WIN32 */
# include <arpa/inet.h>
#endif /* ifdef _WIN32 */
#include "./mm_manager.h"

/* Smart Card Rebate Strings */
const char *str_smcard_rebate[] = {
    "SMART CARD INTRALATA REBATE  ",
    "SMART CARD IXL REBATE        ",
    "SMART CARD LOCAL REBATE      ",
    "SMART CARD INTERLATA REBATE  ",
    "SMART CARD DA REBATE         ",
    "SMART CARD 1 800 REBATE      ",
    "LOCAL LMS ADDITIONAL REBATE  ",
    "INTRALATA ADDITIONAL REBATE  ",
    "INTERLATA ADDITIONAL REBATE  ",
    "IXL ADDITIONAL REBATE        ",
    "DA ADDITIONAL REBATE         ",
    "ONE 800 ADDITIONAL REBATE    ",
    "DATAJACK INITIAL SURCHARGE   ",
    "DATAJACK ADDITIONAL SURCHARGE"
};

int mult_lut[4] = { 1, 5, 10, 25 };

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   index;
    int   ret = 0;

    dlog_mt_scard_parm_table_t *psmcard_table;
    uint8_t* load_buffer;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_smcard mm_table_5d.bin [outputfile.bin]\n");
        return -1;
    }

    printf("Nortel Millennium Call Smart Card Table (Table 93) Dump\n\n");

    psmcard_table = (dlog_mt_scard_parm_table_t *)calloc(1, sizeof(dlog_mt_scard_parm_table_t));

    if (psmcard_table == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_scard_parm_table_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(psmcard_table);
        return -ENOENT;
    }

    load_buffer = ((uint8_t*)psmcard_table) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_scard_parm_table_t) - 1, 1, instream) != 1) {
        printf("Error reading SMCARD table.\n");
        free(psmcard_table);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    printf("+--------------------------+\n" \
           "| Idx | DES Key            |\n" \
           "+--------------------------+\n");

    for (index = 0; index < SC_DES_KEY_MAX; index++) {
        printf("|  %2d | 0x%016llx |\n",
               index,
               psmcard_table->des_key[index]);
    }
    printf("+--------------------------+\n");

    printf("\n+-----+------+-----------+\n" \
           "| Idx | Mult | Max Units |\n"   \
           "+-----+------+-----------+\n");

    for (index = 0; index < SC_MULT_MAX_UNIT_MAX; index++) {
        int multiplier = mult_lut[(htons(psmcard_table->mult_max_unit[index]) & SC_MULT_MASK)];
        int max_units  = htons(psmcard_table->mult_max_unit[index]) >> SC_MAX_UNIT_SHIFT;

        printf("|  %2d |  %2d  |     %5d |\n", index, multiplier, max_units);
    }

    printf("+------------------------+\n");

    printf("\n+-----+-------------------------------+--------+\n" \
           "| Idx | Rebate Type                   | Rebate |\n"   \
           "+-----+-------------------------------+--------+\n");

    for (index = 0; index < SC_REBATE_MAX; index++) {
        int rebate = psmcard_table->rebates[index];

        printf("|  %2d | %s |  $%3.2f |\n", index, str_smcard_rebate[index], (float)rebate / 100);
    }

    printf("+----------------------------------------------+\n");

    if (argc > 2) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            return -ENOENT;
        }
    }

    /* Modify SMCARD table */
    psmcard_table->des_key[0] = 0xFFFFFFFFFFFFFFFFULL;
    psmcard_table->des_key[1] = 0ULL;
    psmcard_table->des_key[2] = 0x7F7F7F7F7F7F7F7FULL;
    psmcard_table->des_key[3] = 0xFEFEFEFEFEFEFEFEULL;

    psmcard_table->mult_max_unit[0] = htons(20  << 2 | 0x01);   // 20
    psmcard_table->mult_max_unit[1] = htons(60  << 2 | 0x01);   // 60
    psmcard_table->mult_max_unit[2] = htons(105 << 2 | 0x01);   // 105
    psmcard_table->mult_max_unit[3] = htons(220 << 2 | 0x01);   // 220
    psmcard_table->mult_max_unit[4] = htons(440 << 2 | 0x01);   // 440
    psmcard_table->mult_max_unit[5] = htons(680 << 2 | 0x01);   // 680
    psmcard_table->mult_max_unit[6] = htons(0   << 2 | 0x01);   // 0
    psmcard_table->mult_max_unit[7] = htons(100 << 2 | 0x01);   // 100
    psmcard_table->mult_max_unit[8] = htons(200 << 2 | 0x01);   // 200
    psmcard_table->mult_max_unit[9] = htons(400 << 2 | 0x01);   // 400

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(load_buffer, sizeof(dlog_mt_scard_parm_table_t) - 1, 1, ostream) != 1) {
            printf("Error writing output file %s\n", argv[2]);
            ret = -EIO;
        }
        fclose(ostream);
    }

    if (psmcard_table != NULL) {
        free(psmcard_table);
    }

    return ret;
}
