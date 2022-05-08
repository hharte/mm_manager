/*
 * Code to dump the Call Screening List table from Nortel Millennium Payphone
 *
 * Table 24 (0x18) - 60-entry DLOG_MT_CALLSCRN_UNIVERSAL
 * Table 92 (0x5c) - 180-entry DLOG_MT_CALL_SCREEN_LIST
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "./mm_manager.h"

#define CALLSCRNU_TABLE_LEN     (sizeof(dlog_mt_call_screen_universal_t))
#define CALLSCRN_TABLE_LEN      (sizeof(dlog_mt_call_screen_list_t))

const char *callscrn_free_flags_str[] = {
    "FREE",  // "FREE_DENY_IND",
    "ASUP",  // "ANSWER_SUPERVISION",
    "KPEN",  // "KEYPAD_ENABLE_IND",
    "POTS",  // "FAIL_TO_POTS_ONLY",
    "MUTE",  // "TRANSMITTER_MUTED",
    "TOUT",  // "TIMEOUT_BEFORE_DIALING",
    "COIN",  // "DENY_CARD_PYMNT_IND", // Coin only, no card
    "FG-B"   // "FEATURE_GROUP_B_NUMBER"
};

/* IDENT2 Flags, see pp. 2-193 */
const char *callscrn_ident2_str[] = {
    "DENY MDS",     // Deny to Message Delivery Service (MDS)
    "DENY DJ",      // Deny to Datajack
    "ALW RES SVC",  // Allow in restricted service
    "TGT CDR REC",  // Targeted CDR Recording
    "PFX Enb",
    "Emrg#Enb",
    "XLAT FLAG",    // TRANSLATION FLAG
    "Reserved"
};

/* Call Class Strings */
const char *str_call_class[] = {
    "Free Call        ",  /* 0x01 */
    "Unknown 0x11     ",  /* 0x11 */
    "Information      ",  /* 0x41 */
    "Operator         ",  /* 0x81 */
    "Disabled         "   /* 0x00 */
};

/* Call Type (lower 4-bits) of CALLTYP */
const char *call_type_strings[16] = {
    "Incomng",  // Incoming
    "Unans'd",  // Unanswered
    "Abandon",  // Abandoned
    "Local  ",  // Local
    "Intra-L",  // Intra-LATA
    "Inter-L",  // Inter-LATA
    "Int'l  ",  // Internatonal
    "Operatr",  // Operator
    "   0+  ",  // Zero-Plus
    " 1-800 ",  // 1-800
    "   DA  ",  // Directory Assistance
    "Denied ",  // Denied
    "Unassgn",  // Unassigned
    "Unassg2",  // Unassigned2
    "e-Purse",  // e-Purse
    "Unknown"   // Unknown
};

uint8_t call_class_lut[] = { 0x01, 0x11, 0x41, 0x81, 0x00 };

int main(int argc, char *argv[]) {
    FILE  *instream;
    int    callscrn_index;
    char   phone_number_str[20] = { 0 };
    int    i;
    int    callscrn_max_entries = 0;
    int    phone_num_len = 0;
    size_t size;

    dlog_mt_call_screen_list_t *pcallscrn_table;
    dlog_mt_call_screen_universal_t* pcallscrnu_table;
    uint8_t* load_buffer;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_callscrn mm_table_18.bin\n"
               "\tmm_callscrn mm_table_5c.bin\n");
        return -1;
    }

    printf("Nortel Millennium Call Screening List Table Dump\n\n");

    pcallscrn_table = (dlog_mt_call_screen_list_t *)calloc(1, sizeof(dlog_mt_call_screen_list_t));
    pcallscrnu_table = (dlog_mt_call_screen_universal_t *)pcallscrn_table;

    if (pcallscrn_table == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_call_screen_list_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(pcallscrn_table);
        return -ENOENT;
    }

    fseek(instream, 0, SEEK_END);
    size = ftell(instream);
    fseek(instream, 0, SEEK_SET);

    switch (size + 1) {
    case CALLSCRNU_TABLE_LEN:
        printf("Call Screen Universal (60-entry) table.\n");
        callscrn_max_entries = CALLSCRNU_TABLE_MAX;
        phone_num_len = 8;
        break;
    case CALLSCRN_TABLE_LEN:
        printf("Call Screening List (180-entry) table.\n");
        callscrn_max_entries = CALLSCRN_TABLE_MAX;
        phone_num_len = 9;
        break;
    default:
        printf("Invalid Call Screening table, len=%zd.\n", size);
        free(pcallscrn_table);
        fclose(instream);
        return -EIO;
    }

    load_buffer = ((uint8_t*)pcallscrn_table) + 1;
    if (fread(load_buffer, size, 1, instream) != 1) {
        printf("Error reading CALLSCRN table.\n");
        free(pcallscrn_table);
        fclose(instream);
        return -EIO;
    }

    printf("+-------------------------------------------------------------------------------------------+\n" \
           "| Call Entry | FCF  |CALLTYP|Carrier|Flags2| Phone Number       | Class | Class Description |\n" \
           "+------------+------+-------+-------+------+--------------------+-------+-------------------+\n");

    for (callscrn_index = 0; callscrn_index < callscrn_max_entries; callscrn_index++) {
        call_screen_list_entry_t *pcallscreen_entry;

        if (size + 1 == CALLSCRN_TABLE_LEN) {
            pcallscreen_entry = &pcallscrn_table->entry[callscrn_index];
        }
        else {
            pcallscreen_entry = (call_screen_list_entry_t *)&pcallscrnu_table->entry[callscrn_index];
        }

        if (pcallscreen_entry->phone_number[0] == 0) continue;

        *phone_number_str = *callscrn_num_to_string(phone_number_str, sizeof(phone_number_str),
                                                    pcallscreen_entry->phone_number, phone_num_len);

        printf("| %3d (0x%02x) | 0x%02x |%s|  0x%02x | 0x%02x | %18s |  0x%02x | ",
               callscrn_index + 1, callscrn_index + 1,
               pcallscreen_entry->free_call_flags,
               call_type_strings[(pcallscreen_entry->call_type) & 0x0F],
               pcallscreen_entry->carrier_ref,
               pcallscreen_entry->ident2,
               phone_number_str,
               pcallscreen_entry->cs_class);

        for (i = 0; i < sizeof(call_class_lut); i++) {
            if (pcallscreen_entry->cs_class == call_class_lut[i]) {
                printf("%s | ", str_call_class[i]);
                break;
            }
        }
        print_bits(pcallscreen_entry->free_call_flags, (char **)callscrn_free_flags_str);
        print_bits(pcallscreen_entry->ident2,          (char **)callscrn_ident2_str);
        printf("\n");
    }

    if (pcallscrn_table != NULL) {
        free(pcallscrn_table);
    }

    printf("+-------------------------------------------------------------------------------------------+\n");

    return 0;
}
