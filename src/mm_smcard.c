/*
 * Code to dump DLOG_MT_SCARD_PARM_TABLE table from Nortel Millennium Payphone
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2023, Howard M. Harte
 *
 */

#include <inttypes.h>
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

#include "mm_manager.h"

#define TABLE_ID    DLOG_MT_SCARD_PARM_TABLE

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

    dlog_mt_scard_parm_table_t *ptable;
    uint8_t* load_buffer;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_smcard mm_table_%02x.bin [outputfile.bin]\n", TABLE_ID);
        return -1;
    }

    printf("Nortel Millennium %s Table %d (0x%02x) Dump\n\n", table_to_string(TABLE_ID), TABLE_ID, TABLE_ID);

    ptable = (dlog_mt_scard_parm_table_t *)calloc(1, sizeof(dlog_mt_scard_parm_table_t));

    if (ptable == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_scard_parm_table_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(ptable);
        return -ENOENT;
    }

    load_buffer = ((uint8_t*)ptable) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_scard_parm_table_t) - 1, 1, instream) != 1) {
        printf("Error reading %s table.\n", table_to_string(TABLE_ID));
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    printf("+--------------------------+\n" \
           "| Idx | DES Key            |\n" \
           "+--------------------------+\n");

    for (index = 0; index < SC_DES_KEY_MAX; index++) {
        printf("|  %2d | 0x%016" PRIx64 " |\n",
               index,
               ptable->des_key[index]);
    }
    printf("+--------------------------+\n");

    printf("\n+-----+------+-----------+\n" \
           "| Idx | Mult | Max Units |\n"   \
           "+-----+------+-----------+\n");

    for (index = 0; index < SC_MULT_MAX_UNIT_MAX; index++) {
        int multiplier = mult_lut[(htons(ptable->mult_max_unit[index]) & SC_MULT_MASK)];
        int max_units  = htons(ptable->mult_max_unit[index]) >> SC_MAX_UNIT_SHIFT;

        printf("|  %2d |  %2d  |     %5d |\n", index, multiplier, max_units);
    }

    printf("+------------------------+\n");

    printf("\n+-----+-------------------------------+---------+\n" \
           "| Idx | Rebate Type                   | Rebate  |\n"   \
           "+-----+-------------------------------+---------+\n");

    for (index = 0; index < SC_REBATE_MAX; index++) {
        int rebate = ptable->rebates[index];

        printf("|  %2d | %s | $%6.2f |\n", index, str_smcard_rebate[index], (float)rebate / 100);
    }

    printf("+-----------------------------------------------+\n");

    if (argc > 2) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            free(ptable);
            return -ENOENT;
        }
    }

    /* Modify SMCARD table */
    ptable->des_key[0] = 0xFFFFFFFFFFFFFFFFULL;
    ptable->des_key[1] = 0ULL;
    ptable->des_key[2] = 0x7F7F7F7F7F7F7F7FULL;
    ptable->des_key[3] = 0xFEFEFEFEFEFEFEFEULL;

    ptable->mult_max_unit[0] = htons(20  << 2 | 0x01);   // 20
    ptable->mult_max_unit[1] = htons(60  << 2 | 0x01);   // 60
    ptable->mult_max_unit[2] = htons(105 << 2 | 0x01);   // 105
    ptable->mult_max_unit[3] = htons(220 << 2 | 0x01);   // 220
    ptable->mult_max_unit[4] = htons(440 << 2 | 0x01);   // 440
    ptable->mult_max_unit[5] = htons(680 << 2 | 0x01);   // 680
    ptable->mult_max_unit[6] = htons(0   << 2 | 0x01);   // 0
    ptable->mult_max_unit[7] = htons(100 << 2 | 0x01);   // 100
    ptable->mult_max_unit[8] = htons(200 << 2 | 0x01);   // 200
    ptable->mult_max_unit[9] = htons(400 << 2 | 0x01);   // 400

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(load_buffer, sizeof(dlog_mt_scard_parm_table_t) - 1, 1, ostream) != 1) {
            printf("Error writing output file %s\n", argv[2]);
            ret = -EIO;
        }
        fclose(ostream);
    }

    free(ptable);

    return ret;
}
