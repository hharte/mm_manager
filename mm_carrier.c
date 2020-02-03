/*
 * Code to dump CARRIER table from Nortel Millennium Payphone
 * Table 135 (0x87)
 *
 * www.github.com/hharte/mm_manager
 *
 * (c) 2020, Howard M. Harte
 *
 * Reference: https://wiki.millennium.management/dlog:dlog_mt_carrier_table
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "mm_manager.h"

/* Default Carrier Mapping strings: */
const char *str_default_carrier[] = {
    "PIC Inter-LATA carrier       ",
    "Coin Inter-LATA carrier      ",
    "Creditcard Inter-LATA carrier",
    "PIC Intra-LATA carrier       ",
    "Coin Intra-LATA carrier      ",
    "Creditcard Intra-LATA carrier",
    "PIC Local carrier            ",
    "Coin Local carrier           ",
    "Creditcard Local carrier     "
};

/* Control Byte strings: */
const char *str_cb[] = {
    "CARCD101XXXX",
    "SPEC_PROMPT",
    "COIN_CASH_CD",
    "ALT_BONG_TMO",
    "DLY_AFT_BONG",
    "INTRA_TO_LEC",
    "OUTDIAL_STR",
    "FEAT_GROUP_B"
};

/* Control Byte 2 strings: */
const char *str_cb2[] = {
    "FGB_PROMPT",
    "RM_PFX_LCL",
    "RM_PFX_INTRA",
    "RM_PFX_INTER",
    "RM_PFX_INT'L",
    "RM_PFX_DA",
    "RM_PFX_1800",
    "CB2_SPARE"
};

int main(int argc, char *argv[])
{
    FILE *instream;
    int carrier_index;
    uint16_t carrier_num;
    char display_prompt_string[21];
    uint8_t cb_flags, i;

    dlog_mt_carrier_table_t *pcarrier_table;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_carrier mm_table_87.bin\n");
        return (-1);
    }

    instream = fopen(argv[1], "rb");

    printf("Nortel Millennium CARRIER Table (Table 135) Dump\n");

    pcarrier_table = calloc(1, sizeof(dlog_mt_carrier_table_t));
    if (fread(pcarrier_table, sizeof(dlog_mt_carrier_table_t), 1, instream) <= 0) {
        printf("Error reading CARRIER table.\n");
        if (pcarrier_table != NULL) {
            free(pcarrier_table);
            return (-2);
        }
    }

    printf("\nDefault Carriers:\n");
    for (i = 0; i < DEFAULT_CARRIERS_MAX; i++) {
        printf("\t%d %s = 0x%02x (%3d)\n",
            i,
            str_default_carrier[i],
            pcarrier_table->defaults[i],
            pcarrier_table->defaults[i]);
    }
    printf("\n+---------------------------------------------------------------------------------------------------------------------+\n" \
            "|  # | Ref  | Number | Valid Cards | Display Prompt       |  CB2 |  CB  | FGB Tmr | Int'l | Call Entry | CB2/CB Flags |\n" \
            "+----+------+--------+-------------+----------------------+------+------+---------+-------+------------+--------------+");

    for (carrier_index = 0; carrier_index < CARRIER_TABLE_MAX_CARRIERS; carrier_index++) {

        if (pcarrier_table->carrier[carrier_index].display_prompt[0] >= 0x20) {
            memcpy(display_prompt_string, pcarrier_table->carrier[carrier_index].display_prompt, sizeof(pcarrier_table->carrier[carrier_index].display_prompt));
            display_prompt_string[20] = '\0';
        } else {
            if((pcarrier_table->carrier[carrier_index].carrier_ref == 0) && (pcarrier_table->carrier[carrier_index].call_entry ==0)) {
                continue;
            }
            sprintf(display_prompt_string, "                    ");
        }

        carrier_num = ntohs(pcarrier_table->carrier[carrier_index].carrier_num);

        printf("\n| %2d | 0x%02x | 0x%04x |  0x%08x | %s | 0x%02x | 0x%02x |  %5d  |  0x%02x | 0x%02x   %3d | ",
            carrier_index,
            pcarrier_table->carrier[carrier_index].carrier_ref,
            carrier_num,
            pcarrier_table->carrier[carrier_index].valid_cards,
            display_prompt_string,
            pcarrier_table->carrier[carrier_index].control_byte2,
            pcarrier_table->carrier[carrier_index].control_byte,
            pcarrier_table->carrier[carrier_index].fgb_timer,
            pcarrier_table->carrier[carrier_index].international_accept_flags,
            pcarrier_table->carrier[carrier_index].call_entry,
            pcarrier_table->carrier[carrier_index].call_entry);

        cb_flags = pcarrier_table->carrier[carrier_index].control_byte2;
        i=0;
        while (cb_flags != 0) {
            if (cb_flags & 1) {
                printf("%s ", str_cb2[i]);
            }
            cb_flags >>= 1;
            i++;
        }

        cb_flags = pcarrier_table->carrier[carrier_index].control_byte;
        i=0;
        while (cb_flags != 0) {
            if (cb_flags & 1) {
                printf("%s ", str_cb[i]);
            }
            cb_flags >>= 1;
            i++;
        }

    }

    if (pcarrier_table != NULL) {
        free(pcarrier_table);
    }

    printf("\n+------------------------------------------------------------------------------------------------------+\n");

    return (0);
}
