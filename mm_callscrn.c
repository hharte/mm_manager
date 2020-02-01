/*
 * Code to dump Call Screening List table from Nortel Millennium Payphone
 * Table 92 (0x5c)
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

/* Call Class Strings */
const char *str_call_class[] = {
    "Free Call       ",     /* 0x01 */
    "Unknown 0x11    ",     /* 0x11 */
    "Information     ",     /* 0x41 */
    "Operator        ",     /* 0x81 */
    "Disabled        "      /* 0x00 */
};

uint8_t call_class_lut[] = { 0x01, 0x11, 0x41, 0x81, 0x00 };

int main(int argc, char *argv[])
{
    FILE *instream;
    int callscrn_index;
    char phone_number_str[20];
    int i;

    dlog_mt_call_screen_list_t *pcallscrn_table;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_callscrn mm_table_5c.bin\n");
        return (-1);
    }

    instream = fopen(argv[1], "rb");

    printf("Nortel Millennium Call Screening List Table (Table 92) Dump\n");

    pcallscrn_table = calloc(1, sizeof(dlog_mt_call_screen_list_t));
    if (fread(pcallscrn_table, sizeof(dlog_mt_call_screen_list_t), 1, instream) <= 0) {
        printf("Error reading CALLSCRN table.\n");
        if (pcallscrn_table != NULL) {
            free(pcallscrn_table);
            return (-2);
        }
    }

    printf("\n+-----------------------------------------------------------------------------------------+\n" \
            "| Dec (Hex)  | Pad0 | Pad1 | Pad2 | Pad3 | Phone Number       | Class | Class Description |\n" \
            "+------------+------+------+------+------+--------------------+-------+-------------------+");

    for (callscrn_index = 0; callscrn_index < CALLSCRN_TABLE_MAX; callscrn_index++) {

        if (pcallscrn_table->entry[callscrn_index].phone_number[0] == 0) continue;

        *phone_number_str = *callscrn_num_to_string(phone_number_str, sizeof(phone_number_str),
            pcallscrn_table->entry[callscrn_index].phone_number, 9);

        printf("\n| %3d (0x%02x) | 0x%02x | 0x%02x | 0x%02x | 0x%02x | %18s |  0x%02x | ",
            callscrn_index, callscrn_index,
            pcallscrn_table->entry[callscrn_index].pad[0],
            pcallscrn_table->entry[callscrn_index].pad[1],
            pcallscrn_table->entry[callscrn_index].pad[2],
            pcallscrn_table->entry[callscrn_index].pad[3],
            phone_number_str,
            pcallscrn_table->entry[callscrn_index].class);

        for (i = 0; i < sizeof(call_class_lut); i++) {
            if (pcallscrn_table->entry[callscrn_index].class == call_class_lut[i]) {
                printf(" %s |", str_call_class[i]);
                break;
            }
        }
    }

    if (pcallscrn_table != NULL) {
        free(pcallscrn_table);
    }

    printf("\n+-----------------------------------------------------------------------------------------+\n");

    return (0);
}
