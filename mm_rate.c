/*
 * Code to dump RATE table from Nortel Millennium Payphone
 * Table 73 (0x49)
 *
 * www.github.com/hharte/mm_manager
 *
 * (c) 2020, Howard M. Harte
 *
 * The RATE Table is an array of 1191 bytes.  The first 39 bytes contain
 * unknown data.
 *
 * The remaining 1152 bytes are a set of 128 9-byte rate entries.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mm_manager.h"

char *str_rates[] = {
    "mm_intra_lata     ",
    "lms_rate_local    ",
    "fixed_charge_local",
    "not_available     ",
    "invalid_npa_nxx   ",
    "toll_intra_lata   ",
    "toll_inter_lata   ",
    "mm_inter_lata     ",
    "mm_local          ",
    "      ?09?        ",
    "      ?0a?        ",
    "      ?0b?        ",
    "      ?0c?        ",
};

int main(int argc, char *argv[])
{
    FILE *instream;
    FILE *ostream = NULL;
    unsigned char c;
    int rate_index;
    uint8_t unknown_bytes[39];
    char rate_str_initial[10];
    char rate_str_additional[10];
    dlog_mt_rate_table_t* prate_table;
    rate_table_entry_t* prate;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_rate mm_table_49.bin [outputfile.bin]\n");
        return (-1);
    }

    instream = fopen(argv[1], "rb");

    printf("Nortel Millennium RATE Table (Table 73) Dump\n");

    prate_table = calloc(1, sizeof(dlog_mt_rate_table_t));
    if (fread(prate_table, sizeof(dlog_mt_rate_table_t), 1, instream) <= 0) {
        printf("Error reading RATE table.\n");
        if (prate_table != NULL) {
            free(prate_table);
            fclose(instream);
            return (-2);
        }
    }

    /* Dump unknown 39 bytes at the beginning of the RATE table */
    printf("39 Unknown bytes in beginning of RATE table:\n");
    dump_hex(prate_table->unknown, 39);

    printf("\n+------------+-------------------------+----------------+--------------+-------------------+-----------------+\n" \
           "| Index      | Type                    | Initial Period | Initial Rate | Additional Period | Additional Rate |\n" \
           "+------------+-------------------------+----------------+--------------+-------------------+-----------------+");

    for (rate_index = 0; rate_index < RATE_TABLE_MAX_ENTRIES; rate_index++) {

        prate = &prate_table->r[rate_index];
        if (prate->type ==  0) continue;

        if (prate->type == 0 && prate->initial_charge == 0 && prate->additional_charge == 0 && \
            prate->initial_period == 0 && prate->additional_period == 0) {
                continue;
            }

        if (prate->initial_period & FLAG_PERIOD_UNLIMITED) {
            strcpy(rate_str_initial, "Unlimited");
        } else {
            sprintf(rate_str_initial, "   %5ds", prate->initial_period);
        }
        if (prate->additional_period & FLAG_PERIOD_UNLIMITED) {
            strcpy(rate_str_additional, "Unlimited");
        } else {
            sprintf(rate_str_additional, "   %5ds", prate->additional_period);
        }

        printf("\n| %3d (0x%02x) | 0x%02x %s |      %s |         %3.2f |         %s |            %3.2f |",
            rate_index, rate_index,
            prate->type,
            str_rates[prate->type],
            rate_str_initial,
            (float)prate->initial_charge / 100,
            rate_str_additional,
            (float)prate->additional_charge / 100 );
    }

    printf("\n+------------------------------------------------------------------------------------------------------------+\n");

    if (argc == 3) {
        ostream = fopen(argv[2], "wb");
    }

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);
        fwrite(prate_table, sizeof(dlog_mt_rate_table_t), 1, ostream);
        fclose(ostream);
    }
    if (prate_table != NULL) {
        free(prate_table);
    }

    return (0);
}
