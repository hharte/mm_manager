/*
 * Code to dump CARRIER table from Nortel Millennium Payphone
 * Table 23 (0x17)
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 *
 * Reference: https://wiki.millennium.management/dlog:dlog_mt_carrier_table
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#ifdef _WIN32
# include <winsock.h>
#else  /* ifdef _WIN32 */
# include <arpa/inet.h>
#endif /* ifdef _WIN32 */
#include "./mm_manager.h"

/* Default Carrier Mapping strings
 *
 * See: https://wiki.millennium.management/dlog:dlog_mt_carrier_table
 *
 * PIC = Presubscribed Interexchange Carrier:
 * https://en.wikipedia.org/wiki/Interexchange_carrier#Carrier_identification_code
 */
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

#define CB2_VAL (CB2_REM_CARRIER_PREFIX_ZM_LOCAL |      \
                 CB2_REM_CARRIER_PREFIX_INTRALATA |     \
                 CB2_REM_CARRIER_PREFIX_INTERLATA |     \
                 CB2_REM_CARRIER_PREFIX_INTERNATIONAL | \
                 CB2_REM_CARRIER_PREFIX_DA |            \
                 CB2_REM_CARRIER_PREFIX_1800)

carrier_table_entry_mtr1_t new_carriers[] = {
    { /* Carrier 0 */
        .carrier_ref = 0,
        .carrier_num = 0x0000,
        .valid_cards = {0xff, 0x3f, 0x00 },

        //                "                    ",
        .display_prompt = "C0 PIC  Inter-LATA  ",
        .control_byte2 = CB2_VAL,
        .control_byte = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer = 500,
        .spare = 0,
        .call_entry = 0x00,
    },
    { /* Carrier 1 */
        .carrier_ref = 1,
        .carrier_num = 0x0000,
        .valid_cards = {0xff, 0x3f, 0x00 },

        //                "                    ",
        .display_prompt = "C1 Coin Inter-LATA  ",
        .control_byte2 = CB2_VAL,
        .control_byte = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer = 500,
        .spare = 0,
        .call_entry = 0x00,
    },
    { /* Carrier 2 */
        .carrier_ref = 2,
        .carrier_num = 0x0000,
        .valid_cards = {0xff, 0x3f, 0x00 },

        //                "                    ",
        .display_prompt = "C2 Card Inter-LATA  ",
        .control_byte2 = CB2_VAL,
        .control_byte = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer = 500,
        .spare = 0,
        .call_entry = 0x00,
    },
    { /* Carrier 3 */
        .carrier_ref = 3,
        .carrier_num = 0x0000,
        .valid_cards = {0xff, 0x3f, 0x00 },

        //                "                    ",
        .display_prompt = "C3 PIC  Intra-LATA  ",
        .control_byte2 = CB2_VAL,
        .control_byte = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer = 500,
        .spare = 0,
        .call_entry = 0x00,
    },
    { /* Carrier 4 */
        .carrier_ref = 4,
        .carrier_num = 0x0000,
        .valid_cards = {0xff, 0x3f, 0x00 },
        //                "                    ",
        .display_prompt             = "C4 Coin Intra-LATA  ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = 500,
        .spare = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 5 */
        .carrier_ref = 5,
        .carrier_num = 0x0000,
        .valid_cards = {0xff, 0x3f, 0x00 },

        //                "                    ",
        .display_prompt             = "C5 Card Intra-LATA  ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = 500,
        .spare = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 6 */
        .carrier_ref = 6,
        .carrier_num = 0x0000,
        .valid_cards = {0xff, 0x3f, 0x00 },

        //                "                    ",
        .display_prompt             = "C6 PIC  Local       ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = 500,
        .spare = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 7 */
        .carrier_ref = 7,
        .carrier_num = 0x0000,
        .valid_cards = {0xff, 0x3f, 0x00 },

        //                "                    ",
        .display_prompt             = "C7 Coin Local       ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = 500,
        .spare = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 8 */
        .carrier_ref = 8,
        .carrier_num = 0x0000,
        .valid_cards = {0xff, 0x3f, 0x00 },

        //                "                    ",
        .display_prompt             = "C8 Card Local       ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = 500,
        .spare = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 9 */
        .carrier_ref = 9,
        .carrier_num = 0x0000,
        .valid_cards = {0xff, 0x3f, 0x00 },

        //                "                    ",
        .display_prompt             = "CARRIER 9           ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = 500,
        .spare = 0,
        .call_entry                 = 0x00,
    }
};

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   carrier_index;
    uint16_t carrier_num;
    char     display_prompt_string[21];
    uint8_t  i;
    int ret = 0;

    dlog_mt_carrier_table_mtr1_t *pcarrier_table;
    uint8_t* load_buffer;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_carrier mm_table_17.bin [outputfile.bin]\n");
        return -1;
    }

    printf("Nortel Millennium CARRIER Table (Table 23) Dump\n\n");

    pcarrier_table = (dlog_mt_carrier_table_mtr1_t *)calloc(1, sizeof(dlog_mt_carrier_table_mtr1_t));

    if (pcarrier_table == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_carrier_table_mtr1_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(pcarrier_table);
        return -ENOENT;
    }

    load_buffer = ((uint8_t*)pcarrier_table) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_carrier_table_mtr1_t) - 1, 1, instream) != 1) {
        printf("Error reading CARRIER table.\n");
        free(pcarrier_table);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    printf("Default Carriers:\n");

    for (i = 0; i < DEFAULT_CARRIERS_MTR1_MAX; i++) {
        printf("\t%d %s = 0x%02x (%3d)\n",
               i,
               str_default_carrier[i],
               pcarrier_table->defaults[i],
               pcarrier_table->defaults[i]);
    }
    printf("\n+---------------------------------------------------------------------------------------------------------------------+\n" \
           "|  # | Ref  | Number | Valid Cards | Display Prompt       |  CB2 |  CB  | FGB Tmr | Int'l | Call Entry | CB2/CB Flags |\n"   \
           "+----+------+--------+-------------+----------------------+------+------+---------+-------+------------+--------------+");

    for (carrier_index = 0; carrier_index < CARRIER_TABLE_MTR1_MAX_CARRIERS; carrier_index++) {
        if (pcarrier_table->carrier[carrier_index].display_prompt[0] >= 0x20) {
            memcpy(display_prompt_string, pcarrier_table->carrier[carrier_index].display_prompt,
                   sizeof(pcarrier_table->carrier[carrier_index].display_prompt));
            display_prompt_string[20] = '\0';
        } else {
            if ((pcarrier_table->carrier[carrier_index].carrier_ref == 0) &&
                (pcarrier_table->carrier[carrier_index].call_entry == 0)) {
                continue;
            }
            snprintf(display_prompt_string, sizeof(display_prompt_string) - 1, "                    ");
        }

        carrier_num = ntohs(pcarrier_table->carrier[carrier_index].carrier_num);

        printf("\n| %2d | 0x%02x | 0x%04x |   0x%02x%02x%02x  | %s | 0x%02x | 0x%02x |  %5d  |  0x%02x | 0x%02x   %3d | ",
               carrier_index,
               pcarrier_table->carrier[carrier_index].carrier_ref,
               carrier_num,
               pcarrier_table->carrier[carrier_index].valid_cards[2],
               pcarrier_table->carrier[carrier_index].valid_cards[1],
               pcarrier_table->carrier[carrier_index].valid_cards[0],
               display_prompt_string,
               pcarrier_table->carrier[carrier_index].control_byte2,
               pcarrier_table->carrier[carrier_index].control_byte,
               pcarrier_table->carrier[carrier_index].fgb_timer,
               pcarrier_table->carrier[carrier_index].spare,
               pcarrier_table->carrier[carrier_index].call_entry,
               pcarrier_table->carrier[carrier_index].call_entry);

        print_bits(pcarrier_table->carrier[carrier_index].control_byte2, (char **)str_cb2);
        print_bits(pcarrier_table->carrier[carrier_index].control_byte,  (char **)str_cb);
    }

    printf("\n+------------------------------------------------------------------------------------------------------+\n");

    printf("Spare: ");
    for (i = 0; i < sizeof(pcarrier_table->spare); i++) {
        printf("0x%02x, ", pcarrier_table->spare[i]);
    }
    printf("\n");

    if (argc > 2) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            return -ENOENT;
        }
    }

    for (i = 0; i < DEFAULT_CARRIERS_MTR1_MAX; i++) {
        pcarrier_table->defaults[i] = 0;
    }

    memcpy(pcarrier_table->carrier, new_carriers, sizeof(new_carriers));

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(load_buffer, sizeof(dlog_mt_carrier_table_mtr1_t) - 1, 1, ostream) != 1) {
            printf("Error writing output file %s\n", argv[2]);
            ret = -EIO;
        }
        fclose(ostream);
    }

    if (pcarrier_table != NULL) {
        free(pcarrier_table);
    }

    return ret;
}
