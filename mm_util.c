/*
 * This is a "Manager" for the Nortel Millennium payhone.
 *
 * It can provision a Nortel Millennium payphone with Rev 1.0 or 1.3
 * Control PCP.  CDRs, Alarms, and Maintenance Reports can also be
 * retieved.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 */

#include <stdio.h>  /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h> /* String function definitions */
#include <time.h>

#include "./mm_manager.h"

#define POLY 0xa001 /* Polynomial to use for CRC-16 calculation */

/* Calculate CRC-16 checksum using 0xA001 polynomial. */
uint16_t crc16(uint16_t crc, uint8_t *buf, size_t len) {
    while (len--) {
        crc ^= *buf++;
        crc  = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc  = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc  = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc  = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc  = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc  = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc  = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc  = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    }
    return crc;
}

void dump_hex(const uint8_t *data, size_t len) {
    uint8_t  ascii[32] = { 0 };
    uint8_t *pascii    = ascii;

    if (len > 0) {
        size_t i;

        for (i = 0; i < len; i++) {
            if (i % 16 == 0) {
                if (i > 0) {
                    *pascii++ = '\0';
                    printf("%s", ascii);
                }
                printf("\n\t%03zu: ", i);
                pascii = ascii;
            }
            printf("%02x, ", data[i]);

            if ((data[i] >= 0x20) && (data[i] < 0x7F)) {
                *pascii++ = data[i];
            } else {
                *pascii++ = '.';
            }
        }
        *pascii++ = '\0';

        if (strnlen((char *)ascii, sizeof(ascii)) > 0) {
            for (i = 0; i < 16 - strnlen((char *)ascii, sizeof(ascii)); i++) {
                printf("    ");
            }
            printf("%s", ascii);
        }
    }
    printf("\n");
}

/* Convert encoded phone number into string. */
extern char* phone_num_to_string(char *string_buf, size_t string_buf_len, uint8_t *num_buf, size_t num_buf_len) {
    char  *pstr = string_buf;
    size_t i, j;

    j = 0;

    for (i = 0; i < num_buf_len; i++) {
        int pn_digit = num_buf[i] >> 4;

        if (pn_digit == 0xe) break;
        *pstr++ = (pn_digit) + '0';
        j++;

        if (j >= (string_buf_len - 1)) break;

        pn_digit = num_buf[i] & 0x0f;

        if (pn_digit == 0xe) break;
        *pstr++ = (pn_digit) + '0';
        j++;

        if (j >= (string_buf_len - 1)) break;
    }

    *pstr = '\0';
    return string_buf;
}

/* Convert NULL terminated string to packed BCD (0 digits replaced with 0xa) */
extern uint8_t string_to_bcd_a(char *number_string, uint8_t *buffer, uint8_t buff_len) {
    uint8_t i;

    memset(buffer, 0, buff_len);

    for (i = 0; (i < (strlen(number_string)) && (i < (buff_len * 2))); i++) {
        if (i % 2 == 0) {
            if (number_string[i] == '0') {
                buffer[(i >> 1)] = 0xa0;
            } else {
                buffer[(i >> 1)] = (number_string[i] - '0') << 4;
            }
        } else {
            if (number_string[i] == '0') {
                buffer[(i >> 1)] |= 0x0a;
            } else {
                buffer[(i >> 1)] |= (number_string[i] - '0');
            }
        }
    }

    return i;
}

/* Lookup table to translate number string into text.  Not sure what B, C, D, E, F are used for. */
const char pn_lut[16] = { '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'B', 'C', 'D', 'E', 'F' };

/* Convert encoded phone number terminated with zero into a string. */
char* callscrn_num_to_string(char *string_buf, size_t string_buf_len, uint8_t *num_buf, size_t num_buf_len) {
    char  *pstr = string_buf;
    size_t i, j;

    j = 0;

    for (i = 0; i < num_buf_len; i++) {
        int pn_digit = num_buf[i] >> 4;

        *pstr++ = pn_lut[pn_digit];
        j++;

        if (j >= (string_buf_len - 1)) break;

        pn_digit = num_buf[i] & 0x0f;
        *pstr++  = pn_lut[pn_digit];
        j++;

        if (j >= (string_buf_len - 1)) break;
    }

    *pstr = '\0';
    return string_buf;
}

/* Convert seconds to DDHHMMSS. */
char* seconds_to_ddhhmmss_string(char* string_buf, size_t string_buf_len, uint32_t seconds) {
    int days, hours, minutes, secs;

    secs = seconds;
    days = secs / (3600 * 24);
    secs -= days * (3600 * 24);
    hours = secs / 3600;
    secs -= hours * 3600;
    minutes = secs / 60;
    secs -= minutes * 60;

    snprintf(string_buf, string_buf_len, "%02d:%02d:%02d:%02d", days, hours, minutes, secs);

    return string_buf;
}

/* Call Type (lower 4-bits) of CALLTYP */
const char *call_type_str[16] = {
    "Incoming",
    "Unanswered",
    "Abandoned",
    "Local",
    "Intra-LATA",
    "Inter-LATA",
    "Internatonal",
    "Operator",
    "Zero+",
    "1-800",
    "Directory Assistance",
    "Denied",
    "Unassigned",
    "Unassigned2",
    "e-Purse",
    "Unknown"
};

/* Payment Type (upper 4-bits) of CALLTYP */
const char *pmt_type_str[16] = {
    "Unused0",
    "Unused1",
    "No Charge",
    "Coin",
    "Credit Card",
    "Calling Card",
    "Cash Card",
    "Inmate",
    "Mondex",
    "Visa Stored Value",
    "Smart City",
    "Proton",
    "UndefinedC",
    "UndefinedD",
    "UndefinedE",
    "UndefinedF",
};

char* call_type_to_string(uint8_t call_type, char *string_buf, size_t string_buf_len) {
    size_t len_call_type, len_pmt_type;

    len_call_type = strlen(call_type_str[call_type & 0x0f]);
    len_pmt_type  = strlen(pmt_type_str[call_type >> 4]);

    if ((len_call_type + len_pmt_type + 1) > string_buf_len) {
        return NULL;
    }

    snprintf(string_buf, string_buf_len, "%s %s",
             call_type_str[call_type & 0x0f],
             pmt_type_str[call_type >> 4]);

    return string_buf;
}

char* timestamp_to_string(uint8_t *timestamp, char *string_buf, size_t string_buf_len) {
    snprintf(string_buf, string_buf_len, "%04d-%02d-%02d %02d:%02d:%02d",
             timestamp[0] + 1900,
             timestamp[1],
             timestamp[2],
             timestamp[3],
             timestamp[4],
             timestamp[5]);

    return string_buf;
}

char* timestamp_to_db_string(uint8_t *timestamp, char *string_buf, size_t string_buf_len) {
    snprintf(string_buf, string_buf_len, "%04d%02d%02d,%02d%02d%02d",
             timestamp[0] + 1900,
             timestamp[1],
             timestamp[2],
             timestamp[3],
             timestamp[4],
             timestamp[5]);

    return string_buf;
}

char* received_time_to_db_string(char *string_buf, size_t string_buf_len) {
    time_t rawtime;
    struct tm ptm = { 0 };

    time(&rawtime);
    localtime_r(&rawtime, &ptm);
    strftime(string_buf, string_buf_len, "%Y%m%d,%H%M%S", &ptm);
    return string_buf;
}

void print_bits(uint8_t bits, char *str_array[]) {
    int i = 0;

    while (bits) {
        if (bits & 1) {
            printf("%s | ", str_array[i]);
        }
        bits >>= 1;
        i++;
    }
}

const char *table_string[] = {
    "INVALID",                      // 0x00
    "DLOG_MT_CARD_AUTH_REQ",        // 0x01
    "DLOG_MT_FUNF_CARD_AUTH",       // 0x02
    "DLOG_MT_AUTH_RESPONSE",        // 0x03
    "DLOG_MT_CD_CALL_DETAILS",      // 0x04
    "DLOG_MT_CDR_DETAILS_ACK",      // 0x05
    "DLOG_MT_MAINT_REQ",            // 0x06
    "DLOG_MT_ALARM",                // 0x07
    "DLOG_MT_CALL_IN",              // 0x08
    "DLOG_MT_CALL_BACK",            // 0x09
    "DLOG_MT_TERM_STATUS",          // 0x0a
    "DLOG_MT_CD_CALL_STATS",        // 0x0b
    "DLOG_MT_PERF_STATS",           // 0x0c
    "DLOG_MT_END_DATA",             // 0x0d
    "DLOG_MT_TAB_UPD_ACK",          // 0x0e
    "DLOG_MT_MAINT_ACK",            // 0x0f
    "DLOG_MT_ALARM_ACK",            // 0x10
    "DLOG_MT_TRANS_DATA",           // 0x11
    "DLOG_MT_TABLE_UPD",            // 0x12
    "DLOG_MT_CALL_BACK_REQ",        // 0x13
    "DLOG_MT_TIME_SYNC",            // 0x14
    "DLOG_MT_NCC_TERM_PARAMS",      // 0x15
    "DLOG_MT_CARD_TABLE",           // 0x16
    "DLOG_MT_CARRIER_TABLE",        // 0x17
    "DLOG_MT_CALLSCRN_UNIVERSAL",   // 0x18
    "0x19",                         // 0x19
    "DLOG_MT_FCONFIG_OPTS",         // 0x1a
    "DLOG_MT_VIS_PROMPTS_L1",       // 0x1b
    "DLOG_MT_VIS_PROMPTS_L2",       // 0x1c
    "DLOG_MT_ADVERT_PROMPTS",       // 0x1d
    "DLOG_MT_USER_IF_PARMS",        // 0x1e
    "DLOG_MT_INSTALL_PARAMS",       // 0x1f
    "DLOG_MT_COMM_STAT_PARMS",      // 0x20
    "DLOG_MT_MODEM_PARMS",          // 0x21
    "DLOG_MT_CALL_STAT_PARMS",      // 0x22
    "DLOG_MT_CALL_IN_PARMS",        // 0x23
    "DLOG_MT_TIME_SYNC_REQ",        // 0x24
    "DLOG_MT_PERF_STATS_MSG",       // 0x25
    "DLOG_MT_CASH_BOX_STATUS",      // 0x26
    "DLOG_MT_ATN_CALL_BACK",        // 0x27
    "DLOG_MT_ATN_REQ_TYPE_40",      // 0x28
    "0x29",                         // 0x29
    "DLOG_MT_ATN_REQ_CDR_UPL",      // 0x2a
    "0x2b",                         // 0x2b
    "DLOG_MT_ATN_REQ_TAB_UPD",      // 0x2c
    "0x2d",                         // 0x2d
    "0x2e",                         // 0x2e
    "0x2f",                         // 0x2f
    "0x30",                         // 0x30
    "DLOG_MT_ANS_SUP_PARAMS",       // 0x31
    "DLOG_MT_COIN_VAL_TABLE",       // 0x32
    "DLOG_MT_CASH_BOX_COLLECTION",  // 0x33
    "0x34",                         // 0x34
    "DLOG_MT_CALL_DETAILS",         // 0x35
    "0x36",                         // 0x36
    "DLOG_MT_REP_DIAL_LIST",        // 0x37
    "DLOG_MT_SUMMARY_CALL_STATS",   // 0x38
    "DLOG_MT_CARRIER_CALL_STATS",   // 0x39
    "DLOG_MT_LIMSERV_DATA",         // 0x3a
    "DLOG_MT_CALLSCRN_EXP",         // 0x3b
    "DLOG_MT_SW_VERSION",           // 0x3c
    "DLOG_MT_COIN_CALL_DETAILS",    // 0x3d
    "DLOG_MT_NUM_PLAN_TABLE",       // 0x3e
    "DLOG_MT_RATE_REQUEST",         // 0x3f
    "DLOG_MT_RATE_RESPONSE",        // 0x40
    "DLOG_MT_AUTH_RESP_CODE",       // 0x41
    "0x42",                         // 0x42
    "0x43",                         // 0x43
    "DLOG_MT_MDS_FCONFIG",          // 0x44
    "DLOG_MT_MDS_STATS",            // 0x45
    "DLOG_MT_MONDEX_DEPOSIT_REC",   // 0x46
    "DLOG_MT_CARRIER_STATS_EXP",    // 0x47
    "DLOG_MT_SPARE_TABLE",          // 0x48
    "DLOG_MT_RATE_TABLE",           // 0x49
    "DLOG_MT_LCD_TABLE_1",          // 0x4a
    "DLOG_MT_LCD_TABLE_2",          // 0x4b
    "DLOG_MT_LCD_TABLE_3",          // 0x4c
    "DLOG_MT_LCD_TABLE_4",          // 0x4d
    "DLOG_MT_LCD_TABLE_5",          // 0x4e
    "DLOG_MT_LCD_TABLE_6",          // 0x4f
    "DLOG_MT_LCD_TABLE_7",          // 0x50
    "DLOG_MT_LCD_TABLE_8",          // 0x51
    "DLOG_MT_QUERY_TERM_ERR",       // 0x52
    "DLOG_MT_TERM_ERR_REP",         // 0x53
    "DLOG_MT_SERIAL_NUM",           // 0x54
    "DLOG_MT_EXP_VIS_PROPTS_L1",    // 0x55
    "DLOG_MT_EXP_VIS_PROPTS_L2",    // 0x56
    "0x57",                         // 0x57
    "0x58",                         // 0x58
    "0x59",                         // 0x59
    "DLOG_MT_LCD_TABLE_9",          // 0x5a
    "DLOG_MT_LCD_TABLE_10",         // 0x5b
    "DLOG_MT_CALL_SCREEN_LIST",     // 0x5c
    "DLOG_MT_SCARD_PARM_TABLE",     // 0x5d
    "DLOG_MT_CODE_DOWNLOAD",        // 0x5e
    "0x5f",                         // 0x5f
    "0x60",                         // 0x60
    "0x61",                         // 0x61
    "0x62",                         // 0x62
    "0x63",                         // 0x63
    "0x64",                         // 0x64
    "DLOG_MT_COMP_LCD_TABLE_1",     // 0x65
    "DLOG_MT_COMP_LCD_TABLE_2",     // 0x66
    "DLOG_MT_COMP_LCD_TABLE_3",     // 0x67
    "DLOG_MT_COMP_LCD_TABLE_4",     // 0x68
    "DLOG_MT_COMP_LCD_TABLE_5",     // 0x69
    "DLOG_MT_COMP_LCD_TABLE_6",     // 0x6a
    "DLOG_MT_COMP_LCD_TABLE_7",     // 0x6b
    "DLOG_MT_COMP_LCD_TABLE_8",     // 0x6c
    "DLOG_MT_COMP_LCD_TABLE_9",     // 0x6d
    "DLOG_MT_COMP_LCD_TABLE_10",    // 0x6e
    "DLOG_MT_COMP_LCD_TABLE_11",    // 0x6f
    "DLOG_MT_COMP_LCD_TABLE_12",    // 0x70
    "DLOG_MT_COMP_LCD_TABLE_13",    // 0x71
    "DLOG_MT_COMP_LCD_TABLE_14",    // 0x72
    "DLOG_MT_COMP_LCD_TABLE_15",    // 0x73
    "0x74",                         // 0x74
    "0x75",                         // 0x75
    "0x76",                         // 0x76
    "0x77",                         // 0x77
    "0x78",                         // 0x78
    "0x79",                         // 0x79
    "0x7a",                         // 0x7a
    "0x7b",                         // 0x7b
    "0x7c",                         // 0x7c
    "0x7d",                         // 0x7d
    "0x7e",                         // 0x7e
    "0x7f",                         // 0x7f
    "0x80",                         // 0x80
    "0x81",                         // 0x81
    "0x82",                         // 0x82
    "0x83",                         // 0x83
    "0x84",                         // 0x84
    "0x85",                         // 0x85
    "DLOG_MT_CARD_TABLE_EXP",       // 0x86
    "DLOG_MT_CARRIER_TABLE_EXP",    // 0x87
    "DLOG_MT_NPA_NXX_TABLE_1",      // 0x88
    "DLOG_MT_NPA_NXX_TABLE_2",      // 0x89
    "DLOG_MT_NPA_NXX_TABLE_3",      // 0x8a
    "DLOG_MT_NPA_NXX_TABLE_4",      // 0x8b
    "DLOG_MT_NPA_NXX_TABLE_5",      // 0x8c
    "DLOG_MT_NPA_NXX_TABLE_6",      // 0x8d
    "DLOG_MT_NPA_NXX_TABLE_7",      // 0x8e
    "DLOG_MT_NPA_NXX_TABLE_8",      // 0x8f
    "DLOG_MT_NPA_NXX_TABLE_9",      // 0x90
    "DLOG_MT_NPA_NXX_TABLE_10",     // 0x91
    "DLOG_MT_NPA_NXX_TABLE_11",     // 0x92
    "DLOG_MT_NPA_NXX_TABLE_12",     // 0x93
    "DLOG_MT_NPA_NXX_TABLE_13",     // 0x94
    "DLOG_MT_NPA_NXX_TABLE_14",     // 0x95
    "DLOG_MT_NPA_SBR_TABLE",        // 0x96
    "DLOG_MT_INTL_SBR_TABLE",       // 0x97
    "DLOG_MT_DISCOUNT_TABLE",       // 0x98
    "0x99",                         // 0x99
    "DLOG_MT_NPA_NXX_TABLE_15",     // 0x9a
    "DLOG_MT_NPA_NXX_TABLE_16",     // 0x9b
    "0x9c",                         // 0x9c
    "0x9d",                         // 0x9d
    "0x9e",                         // 0x9e
    "0x9f",                         // 0x9f
    "DLOG_MT_NPA_NXX_TABLE_15",     // 0xa0
    "DLOG_MT_NPA_NXX_TABLE_16",     // 0xa1
    "0xa2",                         // 0xa2
    "0xa3",                         // 0xa3
    "0xa4",                         // 0xa4
    "0xa5",                         // 0xa5
    "0xa6",                         // 0xa6
    "0xa7",                         // 0xa7
    "0xa8",                         // 0xa8
    "0xa9",                         // 0xa9
    "0xaa",                         // 0xaa
    "0xab",                         // 0xab
    "0xac",                         // 0xac
    "0xad",                         // 0xad
    "0xae",                         // 0xae
    "0xaf"                          // 0xaf
};

const char* table_to_string(uint8_t table) {
    if (table >= (sizeof(table_string) / sizeof(char *))) {
        table = 0;
    }

    return table_string[table];
}

/* Millennium Alarm Strings */
const char* alarm_type_str[] = {
    /* 0 */  "Telephony Board Not Responding",  // TSTATUS_HANDSET_DISCONT_IND
    /* 1 */  "TELEPHONY_STATUS_IND",
    /* 2 */  "EPM/SAM Not Responding",
    /* 3 */  "EPM/SAM Locked Out",
    /* 4 */  "EPM/SAM Expired",
    /* 5 */  "EPM/SAM has reached the transaction limit",
    /* 6 */  "Unable to Reach Primary Collection System",
    /* 7 */  "Reserved TELEPHONY_STATUS_BIT_7",
    /* 8 */  "Power Fail",
    /* 9 */  "Display Not Responding",
    /* 10 */  "Voice Synthesis Not Responding",
    /* 11 */  "Unable to Reach Secondary Collection System",
    /* 12 */  "Card Reader Blocked",
    /* 13 */  "Mandatory Table Alarm",
    /* 14 */  "Datajack Port Blocked",
    /* 15 */  "Reserved CTRL_HW_STATUS_BIT_7",
    /* 16 */  "CDR Checksum Error",
    /* 17 */  "Statistics Checksum Error",
    /* 18 */  "Table Checksum Error",
    /* 19 */  "Data Checksum Error",
    /* 20 */  "CDR List Full",
    /* 21 */  "Bad EEPROM",
    /* 22 */  "Control Microprocessor RAM Contents Lost",
    /* 23 */  "Control Microprocessor RAM Defective",
    /* 24 */  "Station Access Cover Opened",
    /* 25 */  "Stuck Button",
    /* 26 */  "Set Removal",  /* Not all terminals have this switch sensor */
    /* 27 */  "Cash Box Threshold Met",
    /* 28 */  "Coin Box Cover Opened",
    /* 29 */  "Cash Box Removed",
    /* 30 */  "Cash Box Full",
    /* 31 */  "Validator Jam",
    /* 32 */  "Escrow Jam",
    /* 33 */  "Coin Hardware Jam",
    /* 34 */  "Central Office Line Check Failure",
    /* 35 */  "Dialog Failure",
    /* 36 */  "Cash Box Electronic Lock Failure",
    /* 37 */  "Dialog Failure with Collection System",
    /* 38 */  "Code Server Connection Failure",
    /* 39 */  "Code Server Aborted",
    /* ... */
    /* 99  */  "Un-Alarm",
    /* >39 */  "Unknown Alarm!"
};

const char* alarm_id_to_string(uint8_t alarm_id) {
    int alarm_index;

    /* Our Alarm Table only has 41 entries, if alarm is 99 (Un-Alarm) set it to 40,
     * if our alarm is > 39 (last valid alarm, except 99) then use alarm 41 to display
     * "Unknown Alarm."
     */
    if (alarm_id == 99) {
        alarm_index = 40;
    } else if (alarm_id > 39) {
        alarm_index = 41;
    } else {
        alarm_index = alarm_id;
    }

    return alarm_type_str[alarm_index];
}

const char* stats_call_type_str_lut[4] = {
    "Local        ",
    "Inter-LATA   ",
    "Intra-LATA   ",
    "International"
};

const char* stats_call_type_to_str(uint8_t type) {
    if (type >= (sizeof(stats_call_type_str_lut) / sizeof(char*))) {
        type = 0;
    }

    return stats_call_type_str_lut[type];
}

const char* stats_to_str_lut[29] = {
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "InterLATA Coin",
    "11",
    "12",
    "13",
    "14",
    "15",
    "16",
    "17",
    "18",
    "19",
    "20",
    "21",
    "22",
    "23",
    "24",
    "25",
    "26",
    "27",
    "28"
};

const char* stats_to_str(uint8_t type) {
    if (type >= (sizeof(stats_to_str_lut) / sizeof(char*))) {
        type = 0;
    }

    return stats_to_str_lut[type];
}

const char* TCALSTE_stats_to_str_lut[16] = {
    "        Local",
    "   Intra-LATA",
    "   Inter-LATA",
    "International",
    "     Incoming",
    "   Unanswered",
    "    Abandoned",
    "  Oper Assist",
    "           0+",
    "   1-800 Free",
    "       Denied",
    "   Dir Assist",
    "         Free",
    "    Follow-On",
    " Fail to POTS",
    "   Rep Dialer"
};

const char* TCALSTE_stats_to_str(uint8_t type) {
    if (type >= (sizeof(TCALSTE_stats_to_str_lut) / sizeof(char*))) {
        type = 0;
    }

    return TCALSTE_stats_to_str_lut[type];
}

const char* TPERFST_stats_to_str_lut[43] = {
    "Call Attempts",
    "Busy Signal",
    "Call Cleared No Data",
    "No Carrier Detect",
    "CO Access Dial 1",
    "CO Access Dial 2",
    "CO Access Dial 3",
    "CO Access Dial 4",
    "CO Access Dial 5",
    "CO Access Dial 6",
    "CO Access Dial 7",
    "Dial to Carrier 1",
    "Dial to Carrier 2",
    "Dial to Carrier 3",
    "Dial to Carrier 4",
    "Dial to Carrier 5",
    "Dial to Carrier 6",
    "Dial to Carrier 7",
    "Carrier to 1st Packet 1",
    "Carrier to 1st Packet 2",
    "Carrier to 1st Packet 3",
    "Carrier to 1st Packet 4",
    "Carrier to 1st Packet 5",
    "Carrier to 1st Packet 6",
    "Carrier to 1st Packet 7",
    "User Wait to Expect Info 1",
    "User Wait to Expect Info 2",
    "User Wait to Expect Info 3",
    "User Wait to Expect Info 4",
    "User Wait to Expect Info 5",
    "User Wait to Expect Info 6",
    "User Wait to Expect Info 7",
    "Total Dialogs Failed",
    "Packet Received Errors",
    "Packet Retries Received",
    "Inactivity Count",
    "Retry Limit Out of Service",
    "Card Auth Timeouts",
    "Rate Request Timeouts",
    "No Dial Tone",
    "Spare 1",
    "Spare 2",
    "Spare 3"
};

const char* TPERFST_stats_to_str(uint8_t type) {
    if (type >= (sizeof(TPERFST_stats_to_str_lut) / sizeof(char*))) {
        type = 0;
    }

    return TPERFST_stats_to_str_lut[type];
}

const char* error_inject_type_str[] = {
    "No error",
    "CRC Error on Transmit Dialog",
    "CRC Error on Transmit ACK",
    "CRC Error on Receive Dialog",
    "CRC Error on Receive ACK"
};

const char* error_inject_type_to_str(uint8_t type) {
    if (type >= (sizeof(error_inject_type_str) / sizeof(char*))) {
        type = 0;
    }

    return error_inject_type_str[type];
}

const uint16_t term_type_mtr[TERM_TYPE_MAX + 1] = {
    MTR_UNKNOWN,
    MTR_2_X,    /* 1 */
    MTR_2_X,
    MTR_2_X,
    MTR_UNKNOWN,
    MTR_2_X,    /* 5 */
    MTR_1_9,
    MTR_2_X,
    MTR_1_6,
    MTR_1_6,
    MTR_1_13,   /* 10 */
    MTR_1_7,
    MTR_1_7,
    MTR_1_7,
    MTR_1_7,
    MTR_1_7,    /* 15 */
    MTR_1_7,
    MTR_2_X,
    MTR_1_20,
    MTR_1_7,
    MTR_1_20,   /* 20 */
    MTR_1_7,
    MTR_2_X,
    MTR_1_7,
    MTR_1_7,
    MTR_1_7,    /* 25 */
    MTR_1_7,
    MTR_1_9,    /* 27: MTR 1.9 Card-only */
    MTR_1_9,
    MTR_1_9,
    MTR_1_9,    /* 30 */
    MTR_1_9,
    MTR_1_7_INTL,
    MTR_1_9,
    MTR_2_X,
    MTR_2_X,    /* 35 */
    MTR_1_13,   /* NCA1X03, MTR 2.0 appears to use the same tables as 1.13. */
    MTR_1_13,
    MTR_2_X,
    MTR_2_X,
    MTR_2_X,    /* 40 */
    MTR_UNKNOWN,
    MTR_1_9,
    MTR_UNKNOWN,
    MTR_1_10,
    MTR_1_11,   /* 45 */
    MTR_1_9,
    MTR_UNKNOWN,
    MTR_UNKNOWN,
    MTR_UNKNOWN,
    MTR_UNKNOWN,/* 50 */
    MTR_2_X,
    MTR_UNKNOWN,
    MTR_UNKNOWN,
    MTR_UNKNOWN,
    MTR_UNKNOWN,/* 55 */
    MTR_2_X,
    MTR_2_X,
    MTR_1_20,
    MTR_2_X,
    MTR_2_X     /* 60 */
};

uint16_t term_type_to_mtr(uint8_t term_type) {
    if (term_type > 60) return MTR_UNKNOWN;
    return term_type_mtr[term_type];
}

const uint8_t term_type_model[TERM_TYPE_MAX + 1] = {
    TERM_INVALID,
    TERM_COIN_BASIC,    /* 1 */
    TERM_COIN_BASIC,
    TERM_MULTIPAY,
    TERM_MULTIPAY,
    TERM_MULTIPAY,      /* 5 */
    TERM_MULTIPAY,
    TERM_MULTIPAY,
    TERM_INMATE,
    TERM_DESK,
    TERM_MULTIPAY,      /* 10 */
    TERM_DESK,
    TERM_CARD,
    TERM_MULTIPAY,
    TERM_MULTIPAY,
    TERM_DESK,          /* 15 */
    TERM_INMATE,
    TERM_MULTIPAY,
    TERM_COIN_BASIC,
    TERM_COIN_BASIC,
    TERM_MULTIPAY,      /* 20 */
    TERM_MULTIPAY,
    TERM_MULTIPAY,
    TERM_CARD,
    TERM_MULTIPAY,
    TERM_DESK,          /* 25 */
    TERM_MULTIPAY,
    TERM_CARD,
    TERM_MULTIPAY,
    TERM_DESK,
    TERM_MULTIPAY,      /* 30 */
    TERM_MULTIPAY,
    TERM_CARD,
    TERM_MULTIPAY,
    TERM_MULTIPAY,
    TERM_CARD,          /* 35 */
    TERM_MULTIPAY,
    TERM_CARD,
    TERM_COIN_BASIC,
    TERM_CARD,
    TERM_MULTIPAY,      /* 40 */
    TERM_MULTIPAY,
    TERM_COIN_BASIC,
    TERM_MULTIPAY,
    TERM_MULTIPAY,
    TERM_DESK,          /* 45 */
    TERM_MULTIPAY,
    TERM_MULTIPAY,
    TERM_MULTIPAY,
    TERM_MULTIPAY,
    TERM_MULTIPAY,      /* 50 */
    TERM_MULTIPAY,
    TERM_MULTIPAY,
    TERM_MULTIPAY,
    TERM_MULTIPAY,
    TERM_MULTIPAY,      /* 55 */
    TERM_COIN_BASIC,
    TERM_MULTIPAY,
    TERM_CARD,
    TERM_MULTIPAY,
    TERM_MULTIPAY       /* 60 */
};

uint8_t term_type_to_model(uint8_t term_type) {
    if (term_type > 60) return MTR_UNKNOWN;
    return term_type_model[term_type];
}

const char* feature_term_type_str_lut[5] = {
    "Invalid  ",
    "Card     ",
    "Universal",
    "Coin     ",
    "Inmate   "
};

const char* feature_term_type_to_str(uint8_t type) {
    if (type >= (sizeof(feature_term_type_str_lut) / sizeof(char*))) {
        type = 0;
    }

    return feature_term_type_str_lut[type];
}

const char* instsv_flags_bits_str[] = {
    "INSTSV_PREDIAL_ENABLE",
    "INSTSV_PREDIAL_ENABLE_1P",
    "INSTSV_PREDIAL_ENABLE_IXL",
    "INSTSV_PREDIAL_ENABLE_ALL",
    "INSTSV_PREDIAL_ENABLE_PRI_NCC",
    "INSTSV_PREDIAL_ENABLE_SEC_NCC",
    "INSTSV_AMP_ENABLE",
    "INSTSV_RESERVED_7"
};

void print_instsv_table(dlog_mt_install_params_t* instsv_table) {
    char phone_number_string[21];

    printf("             Access code: %s\n",
        phone_num_to_string(phone_number_string, sizeof(phone_number_string),
            instsv_table->access_code, sizeof(instsv_table->access_code)));
    printf("         Key card number: %s\n",
        phone_num_to_string(phone_number_string, sizeof(phone_number_string),
            instsv_table->key_card_number, sizeof(instsv_table->key_card_number)));
    printf("                   flags: 0x%02x\t", instsv_table->flags);
    print_bits(instsv_table->flags, (char**)instsv_flags_bits_str);
    printf("\n");
    printf("         TX Packet Delay: 0x%02x (%dms)\n",
        instsv_table->tx_packet_delay, instsv_table->tx_packet_delay * 10);
    printf("           RX Packet Gap: 0x%02x (%dms)\n",
        instsv_table->rx_packet_gap, instsv_table->rx_packet_gap * 10);
    printf("       Retries until OOS: %d\n", instsv_table->retries_until_oos);
    printf("      Coin service flags: 0x%02x\n", instsv_table->coin_service_flags);
    printf("    Coinbox lock timeout: %d seconds\n", instsv_table->coinbox_lock_timeout);
    printf("          Predial string: %s\n",
        callscrn_num_to_string(phone_number_string, sizeof(phone_number_string),
            instsv_table->predial_string, sizeof(instsv_table->predial_string)));
    printf("Alternate Predial string: %s\n",
        callscrn_num_to_string(phone_number_string, sizeof(phone_number_string),
            instsv_table->predial_string_alt, sizeof(instsv_table->predial_string_alt)));
}

#ifdef _WIN32
char* basename(char* path) {
    char fname[20] = { 0 };

    _splitpath(path, NULL, NULL, fname, NULL);

    snprintf(path, sizeof(fname), "%s", fname);
    return path;
}

errno_t localtime_r(time_t const* const sourceTime, struct tm* tmDest) {
    return localtime_s(tmDest, sourceTime);
}
#endif /* _WIN32 */
