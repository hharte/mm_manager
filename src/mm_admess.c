/*
 * Code to dump DLOG_MT_ADVERT_PROMPTS table from Nortel Millennium Payphone
 * Table 29 (0x1d) - ADMESS pp. 2-3
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

#define TABLE_ID    DLOG_MT_ADVERT_PROMPTS

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   index;
    char  vfd_string[21];
    int   ret = 0;

    dlog_mt_advert_prompts_t *ptable;
    uint8_t *load_buffer;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_admess mm_table_%02x.bin [outputfile.bin]\n", TABLE_ID);
        return -1;
    }

    printf("Nortel Millennium %s Table %d (0x%02x) Dump\n\n", table_to_string(TABLE_ID), TABLE_ID, TABLE_ID);

    ptable = (dlog_mt_advert_prompts_t *)calloc(1, sizeof(dlog_mt_advert_prompts_t));

    if (ptable == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_advert_prompts_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(ptable);
        return -ENOENT;
    }

    if (mm_validate_table_fsize(TABLE_ID, instream, sizeof(dlog_mt_advert_prompts_t) - 1) != 0) {
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    load_buffer = ((uint8_t *)ptable) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_advert_prompts_t) - 1, 1, instream) != 1) {
        printf("Error reading %s table.\n", table_to_string(TABLE_ID));
        fclose(instream);
        free(ptable);
        return -EIO;
    }

    fclose(instream);

    printf("+----------------------------------------------------------+\n" \
           "| Idx  | Duration | Effects | Display Text         | Spare |\n" \
           "+------+----------+---------+----------------------+-------+\n");

    for (index = 0; index < ADVERT_PROMPTS_MAX; index++) {
        memcpy(vfd_string, (char *)ptable->entry[index].message_text, sizeof(vfd_string) - 1);
        vfd_string[sizeof(vfd_string) - 1] = '\0';

        printf("|  %2d  |    %5d |    0x%02x | %s |  0x%02x |\n",
               index,
               LE16(ptable->entry[index].display_time),
               ptable->entry[index].display_attr,
               vfd_string,
               ptable->entry[index].spare);
    }

    printf("+----------------------------------------------------------+\n");

    if (argc > 2) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            return -ENOENT;
        }
    }

    /* Modify ADMESS table */

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(load_buffer, sizeof(dlog_mt_advert_prompts_t) - 1, 1, ostream) != 1) {
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
