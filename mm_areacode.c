/*
 * Code to dump area codes list from Nortel Millennium
 * payphone table 0x96
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
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

const char *str_flags[4] = {
    "---", /* 0 - Invalid NPA */
    "   ", /* 1 - Unassigned */
    "USA", /* 2 - Local, Intra-LATA, Inter-LATA, depending on LCD Table */
    "For"
};

int main(int argc, char *argv[]) {
    FILE *instream;
    int   areacode = 200;
    dlog_mt_npa_sbr_table_t *npa_table;
    uint8_t* load_buffer;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_areacode mm_table_96.bin\n");
        return -1;
    }

    printf("Nortel Millennium NPA (Table 0x96) Dump\n\n");

    npa_table = (dlog_mt_npa_sbr_table_t *)calloc(1, sizeof(dlog_mt_npa_sbr_table_t));

    if (npa_table == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_npa_sbr_table_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(npa_table);
        return -ENOENT;
    }

    load_buffer = ((uint8_t*)npa_table) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_npa_sbr_table_t) - 1, 1, instream) != 1) {
        printf("Error reading NPA table.\n");
        free(npa_table);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    for (int i = 0; i < sizeof(dlog_mt_npa_sbr_table_t) - 1; i++) {
        uint8_t c;
        uint8_t flags0, flags1;

        c = npa_table->npa[i];

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

    if (npa_table != NULL) {
        free(npa_table);
    }

    return 0;
}
