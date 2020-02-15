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
    unsigned char c;
    int rate_index;
    uint8_t unknown_bytes[39];
    char rate_str_initial[10];
    char rate_str_additional[10];

    rate_table_entry_t rate_entry;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_rate mm_table_49.bin\n");
        return (-1);
    }

    instream = fopen(argv[1], "rb");

    printf("Nortel Millennium RATE Table (Table 73) Dump\n");

    /* Skip over unknown 39 bytes at the beginning of the RATE table */
    if (fread(unknown_bytes, 39, 1, instream) > 0) {
        printf("39 Unknown bytes in beginning of RATE table:\n");
        dump_hex(unknown_bytes, 39);
    }

    printf("\n+------------+-------------------------+----------------+--------------+-------------------+-----------------+\n" \
           "| Index      | Type                    | Initial Period | Initial Rate | Additional Period | Additional Rate |\n" \
           "+------------+-------------------------+----------------+--------------+-------------------+-----------------+");

    for (rate_index = 0; ; rate_index++) {

        if (fread(&rate_entry, sizeof(rate_entry), 1, instream) > 0) {

            if (rate_entry.type == 0 && rate_entry.initial_charge == 0 && rate_entry.additional_charge == 0 && \
                rate_entry.initial_period == 0 && rate_entry.additional_period == 0) {
                    continue;
                }

            if (rate_entry.initial_period & FLAG_PERIOD_UNLIMITED) {
                strcpy(rate_str_initial, "Unlimited");
            } else {
                sprintf(rate_str_initial, "   %5ds", rate_entry.initial_period);
            }
            if (rate_entry.additional_period & FLAG_PERIOD_UNLIMITED) {
                strcpy(rate_str_additional, "Unlimited");
            } else {
                sprintf(rate_str_additional, "   %5ds", rate_entry.additional_period);
            }

            printf("\n| %3d (0x%02x) | 0x%02x %s |      %s |         %3.2f |         %s |            %3.2f |",
                rate_index, rate_index,
                rate_entry.type,
                str_rates[rate_entry.type],
                rate_str_initial,
                (float)rate_entry.initial_charge / 100,
                rate_str_additional,
                (float)rate_entry.additional_charge / 100 );
        } else {
            break;
        }
    }

    printf("\n+------------------------------------------------------------------------------------------------------------+\n");
    return 0;
}
