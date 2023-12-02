/*
 * Code to dump DLOG_MT_RATE_TABLE table from Nortel Millennium Payphone
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2023, Howard M. Harte
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
#include <errno.h>

#include "mm_manager.h"

#define TABLE_ID    DLOG_MT_RATE_TABLE

const char *str_rates[] = {
    "mm_intra_lata     ",
    "lms_rate_local    ",
    "fixed_charge_local",
    "not_available     ",
    "invalid_npa_nxx   ",
    "toll_intra_lata   ",
    "toll_inter_lata   ",
    "mm_inter_lata     ",
    "mm_local          ",
    "international     ",
    "      ?0a?        ",
    "      ?0b?        ",
    "      ?0c?        ",
    "      ?0d?        ",
    "      ?0e?        ",
    "      ?0f?        ",
};

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   rate_index;
    char  rate_str_initial[10];
    char  rate_str_additional[10];
    char  timestamp_str[20];
    dlog_mt_rate_table_t *ptable;
    uint8_t *load_buffer;
    size_t size;
    int ret = 0;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_rate mm_table_%02x.bin [outputfile.bin]\n", TABLE_ID);
        return -1;
    }

    printf("Nortel Millennium %s Table %d (0x%02x) Dump\n\n", table_to_string(TABLE_ID), TABLE_ID, TABLE_ID);

    ptable = (dlog_mt_rate_table_t *)calloc(1, sizeof(dlog_mt_rate_table_t));

    if (ptable == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_rate_table_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(ptable);
        return -ENOENT;
    }

    fseek(instream, 0, SEEK_END);
    size = ftell(instream);
    fseek(instream, 0, SEEK_SET);

    if (size != (sizeof(dlog_mt_rate_table_t) - 1)) {
        printf("Incorrect length for %s table, expected: %lu bytes, actual: %zd bytes.\n",
            table_to_string(TABLE_ID),
            sizeof(dlog_mt_rate_table_t) - 1,
            size);
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    load_buffer = ((uint8_t*)ptable) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_rate_table_t) - 1, 1, instream) != 1) {
        printf("Error reading %s table.\n", table_to_string(TABLE_ID));
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    printf("Date: %s\n", timestamp_to_string(ptable->timestamp, timestamp_str, sizeof(timestamp_str)));
    printf("Telco ID: 0x%02x (%d)\n", ptable->telco_id, ptable->telco_id);

    /* Dump spare 32 bytes at the beginning of the RATE table */
    printf("Spare bytes:\n");
    dump_hex(ptable->spare, 32);

    printf("\n+------------+-------------------------+----------------+--------------+-------------------+-----------------+\n" \
           "| Index      | Type                    | Initial Period | Initial Rate | Additional Period | Additional Rate |\n"   \
           "+------------+-------------------------+----------------+--------------+-------------------+-----------------+");

    for (rate_index = 0; rate_index < RATE_TABLE_MAX_ENTRIES; rate_index++) {
        rate_table_entry_t *prate = &ptable->r[rate_index];

        if (prate->type ==  0) continue;

        if ((prate->type == 0) && (LE16(prate->initial_charge) == 0) && (LE16(prate->additional_charge) == 0) && \
            (LE16(prate->initial_period) == 0) && (LE16(prate->additional_period) == 0)) {
            continue;
        }

        if (LE16(prate->initial_period) & FLAG_PERIOD_UNLIMITED) {
            snprintf(rate_str_initial, sizeof(rate_str_initial), "Unlimited");
        } else {
            snprintf(rate_str_initial, sizeof(rate_str_initial), "   %5ds", LE16(prate->initial_period));
        }

        if (LE16(prate->additional_period) & FLAG_PERIOD_UNLIMITED) {
            snprintf(rate_str_additional, sizeof(rate_str_additional), "Unlimited");
        } else {
            snprintf(rate_str_additional, sizeof(rate_str_additional), "   %5ds", LE16(prate->additional_period));
        }

        printf("\n| %3d (0x%02x) | 0x%02x %s |      %s |       %6.2f |         %s |          %6.2f |",
               rate_index, rate_index,
               prate->type,
               str_rates[(prate->type) & 0x0F],
               rate_str_initial,
               (float)LE16(prate->initial_charge) / 100,
               rate_str_additional,
               (float)LE16(prate->additional_charge) / 100);
    }

    printf("\n+------------------------------------------------------------------------------------------------------------+\n");

    /* Modify RATE table */

    if (argc == 3) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            return -ENOENT;
        }
    }

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(load_buffer, sizeof(dlog_mt_rate_table_t) - 1, 1, ostream) != 1) {
            printf("Error writing output file %s\n", argv[2]);
            ret = -EIO;
        }
        fclose(ostream);
    }

    if (ptable != NULL) {
        free(ptable);
    }

    return ret;
}
