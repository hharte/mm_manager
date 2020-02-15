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

const char *callscrn_free_flags_str[] = {
    "FREE", // "FREE_DENY_IND",
    "ASUP", // "ANSWER_SUPERVISION",
    "KPEN", // "KEYPAD_ENABLE_IND",
    "POTS", // "FAIL_TO_POTS_ONLY",
    "MUTE", // "TRANSMITTER_MUTED",
    "TOUT", // "TIMEOUT_BEFORE_DIALING",
    "COIN", // "DENY_CARD_PYMNT_IND", // Coin only, no card
    "FG-B"  // "FEATURE_GROUP_B_NUMBER"
};

/* IDENT2 Flags, see pp. 2-193 */
const char *callscrn_ident2_str[] = {
    "DENY MDS", // Deny to Message Delivery Service (MDS)
    "DENY DJ",  // Deny to Datajack
    "ALW RES SVC",  // Allow in restricted service
    "TGT CDR REC",  // Targeted CDR Recording
    "PFX Enb",
    "Emrg#Enb",
    "XLAT FLAG",    // TRANSLATION FLAG
    "Reserved"
};

/* Call Class Strings */
const char *str_call_class[] = {
    "Free Call        ",     /* 0x01 */
    "Unknown 0x11     ",     /* 0x11 */
    "Information      ",     /* 0x41 */
    "Operator         ",     /* 0x81 */
    "Disabled         "      /* 0x00 */
};

/* Call Type (lower 4-bits) of CALLTYP */
char *call_type_strings[16] = {
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

int main(int argc, char *argv[])
{
    FILE *instream;
    int callscrn_index;
    char phone_number_str[20];
    int i;

    dlog_mt_call_screen_list_t *pcallscrn_table;
    call_screen_list_entry_t *pcallscreen_entry;

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

    printf("\n+-------------------------------------------------------------------------------------------+\n" \
            "| Call Entry | FCF  |CALLTYP|Carrier|Flags2| Phone Number       | Class | Class Description |\n" \
            "+------------+------+-------+-------+------+--------------------+-------+-------------------+\n");

    for (callscrn_index = 0; callscrn_index < CALLSCRN_TABLE_MAX; callscrn_index++) {

        pcallscreen_entry = &pcallscrn_table->entry[callscrn_index];
        if (pcallscreen_entry->phone_number[0] == 0) continue;

        *phone_number_str = *callscrn_num_to_string(phone_number_str, sizeof(phone_number_str),
            pcallscreen_entry->phone_number, 9);

        printf("| %3d (0x%02x) | 0x%02x |%s|  0x%02x | 0x%02x | %18s |  0x%02x | ",
            callscrn_index + 1, callscrn_index + 1,
            pcallscreen_entry->free_call_flags,
            call_type_strings[pcallscreen_entry->call_type],
            pcallscreen_entry->carrier_ref,
            pcallscreen_entry->ident2,
            phone_number_str,
            pcallscreen_entry->class);

        for (i = 0; i < sizeof(call_class_lut); i++) {
            if (pcallscreen_entry->class == call_class_lut[i]) {
                printf("%s | ", str_call_class[i]);
                break;
            }
        }
        print_bits(pcallscreen_entry->free_call_flags, (char **)callscrn_free_flags_str);
        print_bits(pcallscreen_entry->ident2, (char **)callscrn_ident2_str);
        printf("\n");
    }

    if (pcallscrn_table != NULL) {
        free(pcallscrn_table);
    }

    printf("+-------------------------------------------------------------------------------------------+\n");

    return (0);
}
