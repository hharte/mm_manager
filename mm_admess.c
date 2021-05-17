/*
 * Code to dump Advertisiing Message table from Nortel Millennium Payphone
 * Table 29 (0x1d) - ADMESS pp. 2-3
 *
 * www.github.com/hharte/mm_manager
 *
 * (c) 2020, Howard M. Harte
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mm_manager.h"

int main(int argc, char *argv[])
{
    FILE *instream;
    FILE *ostream = NULL;
    int index;
    char vfd_string[21];
    int i;

    dlog_mt_advert_prompts_t *padmess_table;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_admess mm_table_1d.bin [outputfile.bin]\n");
        return (-1);
    }

    printf("Nortel Millennium Advertising Message Table (Table 29) Dump\n");

    padmess_table = calloc(1, sizeof(dlog_mt_advert_prompts_t));
    if (padmess_table == NULL) {
        printf("Failed to allocate %lu bytes.\n", (unsigned long)sizeof(dlog_mt_advert_prompts_t));
        return(-2);
    }

    instream = fopen(argv[1], "rb");

    if (fread(padmess_table, sizeof(dlog_mt_advert_prompts_t), 1, instream) == 0) {
        printf("Error reading ADMESS table.\n");
        fclose(instream);
        free(padmess_table);
        return (-2);
    }

    fclose(instream);

    printf("\n+----------------------------------------------------------+\n" \
            "| Idx  | Duration | Effects | Display Text         | Spare |\n" \
            "+------+----------+---------+----------------------+-------+\n");

    for (index = 0; index < ADVERT_PROMPTS_MAX; index++) {

        snprintf(vfd_string, sizeof(vfd_string), "%s", (char *)padmess_table->entry[index].message_text);

        printf("|  %2d  |    %5d |    0x%02x | %s |  0x%02x |\n",
            index,
            padmess_table->entry[index].display_time,
            padmess_table->entry[index].display_attr,
            vfd_string,
            padmess_table->entry[index].spare);
    }

    printf("+----------------------------------------------------------+\n");

    if (argc > 2) {
        ostream = fopen(argv[2], "wb");
    }

    /* Modify ADMESS table */

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);
        fwrite(padmess_table, sizeof(dlog_mt_advert_prompts_t), 1, ostream);
        fclose(ostream);
    }

    if (padmess_table != NULL) {
        free(padmess_table);
    }

    return (0);
}
