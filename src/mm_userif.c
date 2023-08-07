/*
 * Code to dump DLOG_MT_USER_IF_PARMS table from Nortel Millennium Payphone
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2023, Howard M. Harte
 *
 * Reference: https://wiki.millennium.management/dlog:dlog_mt_carrier_table
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "mm_manager.h"

#define TABLE_ID    DLOG_MT_USER_IF_PARMS

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   ret = 0;

    dlog_mt_user_if_params_t *ptable;
    uint8_t* load_buffer;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_userif mm_table_%02x.bin [outputfile.bin]\n", TABLE_ID);
        return -1;
    }

    printf("Nortel Millennium %s Table %d (0x%02x) Dump\n\n", table_to_string(TABLE_ID), TABLE_ID, TABLE_ID);

    ptable = (dlog_mt_user_if_params_t *)calloc(1, sizeof(dlog_mt_user_if_params_t));

    if (ptable == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_user_if_params_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(ptable);
        return -ENOENT;
    }

    load_buffer = ((uint8_t*)ptable) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_user_if_params_t) - 1, 1, instream) != 1) {
        printf("Error reading %s table.\n", table_to_string(TABLE_ID));
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    print_user_if_params_table(ptable);

    if (argc > 2) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            return -ENOENT;
        }
    }

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(load_buffer, sizeof(dlog_mt_user_if_params_t) - 1, 1, ostream) != 1) {
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
