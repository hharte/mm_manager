/*
 * Code to dump Smartcard table from Nortel Millennium Payphone
 * Table 93 (0x5d)
 *
 * www.github.com/hharte/mm_manager
 *
 * (c) 2020-2022, Howard M. Harte
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mm_manager.h"

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

int mult_lut[4] = {1, 5, 10, 25};

int main(int argc, char *argv[])
{
    FILE *instream;
    int index;

    dlog_mt_scard_parm_table_t *psmcard_table;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_smcard mm_table_5d.bin\n");
        return (-1);
    }

    printf("Nortel Millennium Call Smart Card Table (Table 93) Dump\n");

    psmcard_table = calloc(1, sizeof(dlog_mt_scard_parm_table_t));
    if (psmcard_table == NULL) {
        printf("Failed to allocate %lu bytes.\n", (unsigned long)sizeof(dlog_mt_scard_parm_table_t));
        return(-2);
    }

    instream = fopen(argv[1], "rb");

    if (fread(psmcard_table, sizeof(dlog_mt_scard_parm_table_t), 1, instream) != 1) {
        printf("Error reading SMCARD table.\n");
        free(psmcard_table);
        fclose(instream);
        return (-2);
    }

    printf("\n+--------------------------+\n" \
            "| Idx | DES Key            |\n" \
            "+--------------------------+\n");

    for (index = 0; index < SC_DES_KEY_MAX; index++) {
        printf("|  %2d | 0x%02x%02x%02x%02x%02x%02x%02x%02x |\n",
            index,
            psmcard_table->des_key[index].x[0],
            psmcard_table->des_key[index].x[1],
            psmcard_table->des_key[index].x[2],
            psmcard_table->des_key[index].x[3],
            psmcard_table->des_key[index].x[4],
            psmcard_table->des_key[index].x[5],
            psmcard_table->des_key[index].x[6],
            psmcard_table->des_key[index].x[7]);
    }
    printf("+--------------------------+\n");

    printf("\n+-----+------+-----------+\n" \
           "| Idx | Mult | Max Units |\n" \
           "+-----+------+-----------+\n");
    for (index = 0; index < SC_MULT_MAX_UNIT_MAX; index++) {
        int multiplier = mult_lut[(psmcard_table->mult_max_unit[index] & SC_MULT_MASK) >> SC_MULT_SHIFT];
        int max_units = psmcard_table->mult_max_unit[index] & SC_MAX_UNIT_MASK;

        printf("|  %2d |  %2d  |     %5d |\n", index, multiplier, max_units);
    }

    printf("+------------------------+\n");

    printf("\n+-----+-------------------------------+--------+\n" \
           "| Idx | Rebate Type                   | Rebate |\n" \
           "+-----+-------------------------------+--------+\n");
    for (index = 0; index < SC_REBATE_MAX; index++) {
        int rebate = psmcard_table->rebates[index];

        printf("|  %2d | %s |  $%3.2f |\n", index, str_smcard_rebate[index], (float)rebate / 100);
    }
    if (psmcard_table != NULL) {
        free(psmcard_table);
    }

    printf("+----------------------------------------------+\n");

    return (0);
}
