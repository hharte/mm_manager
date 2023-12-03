/*
 * Code to dump DLOG_MT_LIMSERV_DATA table from Nortel Millennium Payphone
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2023, Howard M. Harte
 *
 * The LIMSERV Table is an array of 26 bytes.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "mm_manager.h"

#define TABLE_ID    DLOG_MT_LIMSERV_DATA

const char *str_restrictions[] = {
    "RESTRICTION_LEVEL_NONE         ",
    "RESTRICTION_LEVEL_EMERGENCY    ",
    "RESTRICTION_LEVEL_DENY_COIN_SC ",
    "RESTRICTION_LEVEL_NO_E2E_SIGNAL",
};

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   limserv_index;
    dlog_mt_limserv_data_t *ptable;
    uint8_t *load_buffer;
    int ret = 0;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_limserv mm_table_%02x.bin [outputfile.bin]\n", TABLE_ID);
        return -1;
    }

    printf("Nortel Millennium %s Table %d (0x%02x) Dump\n\n", table_to_string(TABLE_ID), TABLE_ID, TABLE_ID);

    ptable = (dlog_mt_limserv_data_t *)calloc(1, sizeof(dlog_mt_limserv_data_t));

    if (ptable == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_limserv_data_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(ptable);
        return -ENOENT;
    }

    if (mm_validate_table_fsize(TABLE_ID, instream, sizeof(dlog_mt_limserv_data_t) - 1) != 0) {
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    load_buffer = ((uint8_t*)ptable) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_limserv_data_t) - 1, 1, instream) != 1) {
        printf("Error reading %s table.\n", table_to_string(TABLE_ID));
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    printf("+-------+----------+---------------------------------+");
    printf("\n| Start | Duration | Restriction                     |");
    printf("\n+-------+----------+---------------------------------+");
    for (limserv_index = 0; limserv_index < LIMSERV_DATA_MAX_ENTRIES; limserv_index++) {
        servlev_entry_t *pservlev = &ptable->servlev[limserv_index];

        printf("\n| %02d:%02d | %02d:%02d    | %s |",
               pservlev->start_time[0],
               pservlev->start_time[1],
               pservlev->duration_time[0],
               pservlev->duration_time[1],
               str_restrictions[pservlev->restriction_level]);
    }

    printf("\n+-------+----------+---------------------------------+\n");

    /* Modify LIMSERV table */

    ptable->servlev[0].restriction_level = RESTRICTION_LEVEL_NONE;
    ptable->servlev[0].start_time[0] = 11;
    ptable->servlev[0].start_time[1] = 00;
    ptable->servlev[0].duration_time[0] = 01;
    ptable->servlev[0].duration_time[1] = 00;

    ptable->servlev[1].restriction_level = RESTRICTION_LEVEL_EMERGENCY;
    ptable->servlev[1].start_time[0] = 13;
    ptable->servlev[1].start_time[1] = 00;
    ptable->servlev[1].duration_time[0] = 01;
    ptable->servlev[1].duration_time[1] = 00;

    ptable->servlev[2].restriction_level = RESTRICTION_LEVEL_NO_E2E_SIGNAL;
    ptable->servlev[2].start_time[0] = 15;
    ptable->servlev[2].start_time[1] = 00;
    ptable->servlev[2].duration_time[0] = 02;
    ptable->servlev[2].duration_time[1] = 00;

    ptable->servlev[3].restriction_level = RESTRICTION_LEVEL_NONE;
    ptable->servlev[3].start_time[0] = 17;
    ptable->servlev[3].start_time[1] = 00;
    ptable->servlev[3].duration_time[0] = 02;
    ptable->servlev[3].duration_time[1] = 00;

    ptable->servlev[4].restriction_level = RESTRICTION_LEVEL_EMERGENCY;
    ptable->servlev[4].start_time[0] = 19;
    ptable->servlev[4].start_time[1] = 00;
    ptable->servlev[4].duration_time[0] = 01;
    ptable->servlev[4].duration_time[1] = 00;

    if (argc == 3) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            return -ENOENT;
        }
    }

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(load_buffer, sizeof(dlog_mt_limserv_data_t) - 1, 1, ostream) != 1) {
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
