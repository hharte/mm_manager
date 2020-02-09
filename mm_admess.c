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
    int index;
    char vfd_string[21];
    int i;

    dlog_mt_advert_prompts_t *admess_table;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_admess mm_table_1d.bin\n");
        return (-1);
    }

    instream = fopen(argv[1], "rb");

    printf("Nortel Millennium Advertising Message Table (Table 29) Dump\n");

    admess_table = calloc(1, sizeof(dlog_mt_advert_prompts_t));
    if (fread(admess_table, sizeof(dlog_mt_advert_prompts_t), 1, instream) <= 0) {
        printf("Error reading ADMESS table.\n");
        if (admess_table != NULL) {
            free(admess_table);
            return (-2);
        }
    }

    printf("\n+----------------------------------------------------------+\n" \
            "| Idx  | Duration | Effects | Display Text         | Spare |\n" \
            "+------+----------+---------+----------------------+-------+\n");

    for (index = 0; index < ADVERT_PROMPTS_MAX; index++) {

        strncpy(vfd_string, (char *)admess_table->entry[index].message_text, 20);
        vfd_string[20] = '\0';

        printf("|  %2d  |    %5d |    0x%02x | %s |  0x%02x |\n",
            index,
            admess_table->entry[index].display_time,
            admess_table->entry[index].display_attr,
            vfd_string,
            admess_table->entry[index].spare);
    }

    if (admess_table != NULL) {
        free(admess_table);
    }

    printf("+----------------------------------------------------------+\n");

    return (0);
}
