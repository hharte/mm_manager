/*
 * Convert Call Screening List table for the Nortel Millennium Payphone
 * from MTR 2.x to MTR 1.7 / 1.9.
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

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    uint8_t *load_buffer = NULL;
    uint8_t* save_buffer = NULL;
    int   index;
    int   ret = 0;

    dlog_mt_call_screen_universal_t *pcallscrnu_table;
    dlog_mt_call_screen_list_t *pcallscrn_table;

    if (argc <= 2) {
        printf("Usage:\n" \
               "\tmm_convert_callscrn_mtr2_to_mtr1 mm_table_5c.bin mm_table_18.bin\n");
        return -1;
    }

    printf("Nortel Millennium Call Screen List Table MTR 2 to MTR 1 Converter\n\n");

    pcallscrn_table = (dlog_mt_call_screen_list_t *)calloc(1, sizeof(dlog_mt_call_screen_list_t));

    if (pcallscrn_table == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_call_screen_list_t));
        return -ENOMEM;
    }

    pcallscrnu_table = (dlog_mt_call_screen_universal_t *)calloc(1, sizeof(dlog_mt_call_screen_universal_t));

    if (pcallscrnu_table == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_call_screen_universal_t));
        free(pcallscrn_table);
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(pcallscrn_table);
        free(pcallscrnu_table);
        return -ENOENT;
    }

    load_buffer = ((uint8_t*)pcallscrn_table) + 1;

    if (fread(load_buffer, sizeof(dlog_mt_call_screen_list_t) - 1, 1, instream) != 1) {
        printf("Error reading MTR 2 CALLSCRN table.\n");
        free(pcallscrn_table);
        free(pcallscrnu_table);
        fclose(instream);
        return -EIO;
    }

    for (index = 0; index < CALLSCRNU_TABLE_MAX; index++) {
        call_screen_list_entry_t *pcallscrn_entry = &pcallscrn_table->entry[index];
        call_screen_universal_entry_t *pcallscrnu_entry = &pcallscrnu_table->entry[index];

        /* Copy the card entry */
        memcpy(pcallscrnu_entry, (call_screen_universal_entry_t *)pcallscrn_entry, sizeof(call_screen_universal_entry_t));
    }

    if ((ostream = fopen(argv[2], "wb")) == NULL) {
        printf("Error opening output file %s for write.\n", argv[2]);
        return -ENOENT;
    }

    printf("\nWriting new table to %s\n", argv[2]);

    save_buffer = ((uint8_t*)pcallscrnu_table) + 1;

    if (fwrite(save_buffer, sizeof(dlog_mt_call_screen_universal_t) - 1, 1, ostream) != 1) {
        printf("Error writing output file %s\n", argv[2]);
        ret = -EIO;
    }

    fclose(ostream);
    free(pcallscrn_table);
    free(pcallscrnu_table);

    return ret;
}
