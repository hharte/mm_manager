/*
 * Code to dump DLOG_MT_NPA_SBR_TABLE table from Nortel Millennium
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2023, Howard M. Harte
 *
 * Table 0x96 is an array of 400 bytes, where each nibble corresponds
 * to one of the area codes from 200-999.
 *
 * Each nibble contains a value representing the type of area code
 * as follows:
 * 0 - Invalid (for example, 211, 311, 800, 900)
 * 2 - Unassigned at this time.
 * 4 - US / Canada Area Code.
 * 6 - International Area Code (for example, the Bahamas.)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "./mm_manager.h"

#define TABLE_ID    DLOG_MT_NPA_SBR_TABLE

const char *str_flags[4] = {
    "---", /* 0 - Invalid NPA */
    "   ", /* 1 - Unassigned */
    "USA", /* 2 - Local, Intra-LATA, Inter-LATA, depending on LCD Table */
    "For"
};

int main(int argc, char *argv[]) {
    FILE *instream;
    int   areacode = 200;
    dlog_mt_npa_sbr_table_t *ptable;
    uint8_t* load_buffer;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_areacode mm_table_%02x.bin\n", TABLE_ID);
        return -1;
    }

    printf("Nortel Millennium %s Table %d (0x%02x) Dump\n\n", table_to_string(TABLE_ID), TABLE_ID, TABLE_ID);

    ptable = (dlog_mt_npa_sbr_table_t *)calloc(1, sizeof(dlog_mt_npa_sbr_table_t));

    if (ptable == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_npa_sbr_table_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(ptable);
        return -ENOENT;
    }

    load_buffer = ((uint8_t*)ptable) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_npa_sbr_table_t) - 1, 1, instream) != 1) {
        printf("Error reading %s table.\n", table_to_string(TABLE_ID));
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    for (size_t i = 0; i < sizeof(dlog_mt_npa_sbr_table_t) - 1; i++) {
        uint8_t c;
        uint8_t flags0, flags1;

        c = ptable->npa[i];

        if (areacode % 200 == 0) {
            printf("\n+-----------------------------------------------------------------+\n" \
                   "| NPA |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |\n"   \
                   "+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+");
        }

        if (areacode % 10 == 0) {
            printf("\n| %02dx |", areacode / 10);
        }

        flags0 = (c & 0x70) >> 4;
        flags1 = c & 0x07;

        if (argc < 3) {
            printf(" %s | %s |", str_flags[flags0 >> 1], str_flags[flags1 >> 1]);
        } else {
            printf("  %x  |  %x  |", flags0, flags1);
        }
        areacode += 2;
    }

    printf("\n+-----------------------------------------------------------------+\n");

    if (ptable != NULL) {
        free(ptable);
    }

    return 0;
}
