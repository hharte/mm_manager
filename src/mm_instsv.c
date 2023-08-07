/*
 * Code to dump DLOG_MT_INSTALL_PARAMS table from Nortel Millennium Payphone
 * Table 31 (0x1f) - INSTSV pp. 2-236
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2023, Howard M. Harte
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "mm_manager.h"

#define TABLE_ID    DLOG_MT_INSTALL_PARAMS

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    dlog_mt_install_params_t *instsv_table;
    uint8_t* load_buffer;
    int  ret = 0;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_instsv mm_table_%02x.bin [outputfile.bin]\n", TABLE_ID);
        return -1;
    }

    printf("Nortel Millennium %s Table %d (0x%02x) Dump\n\n", table_to_string(TABLE_ID), TABLE_ID, TABLE_ID);

    instsv_table = (dlog_mt_install_params_t *)calloc(1, sizeof(dlog_mt_install_params_t));

    if (instsv_table == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_install_params_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(instsv_table);
        return -ENOENT;
    }

    load_buffer = ((uint8_t*)instsv_table) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_install_params_t) - 1, 1, instream) != 1) {
        printf("Error reading %s table.\n", table_to_string(TABLE_ID));
        free(instsv_table);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    print_instsv_table(instsv_table);

    /* Modify INSTSV table */
    instsv_table->coin_service_flags = INSTSV_CASHBOX_QUERY_MENU_ENABLE;

    if (argc > 2) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            free(instsv_table);
            return -ENOENT;
        }
    }

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(load_buffer, sizeof(dlog_mt_install_params_t) - 1, 1, ostream) != 1) {
            printf("Error writing output file %s\n", argv[2]);
            ret = -EIO;
        }
        fclose(ostream);
    }

    if (instsv_table != NULL) {
        free(instsv_table);
    }

    return ret;
}
