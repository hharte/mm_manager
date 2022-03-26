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

#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>  /* String function definitions */

#include "./mm_manager.h"

#define POLY 0xa001     /* Polynomial to use for CRC-16 calculation */

/* Calculate CRC-16 checksum using 0xA001 polynomial. */
unsigned crc16(unsigned crc, uint8_t *buf, size_t len) {
    while (len--) {
        crc ^= *buf++;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    }
    return crc;
}

void dump_hex(const uint8_t *data, size_t len) {
    uint8_t ascii[32] = { 0 };
    uint8_t *pascii = ascii;

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
extern char *phone_num_to_string(char *string_buf, size_t string_buf_len, uint8_t* num_buf, size_t num_buf_len) {
    char *pstr = string_buf;
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

    return (i);
}

/* Lookup table to translate number string into text.  Not sure what B, C, D, E, F are used for. */
const char pn_lut[16] = { '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'B', 'C', 'D', 'E', 'F' };

/* Convert encoded phone number terminated with zero into a string. */
char *callscrn_num_to_string(char *string_buf, size_t string_buf_len, uint8_t* num_buf, size_t num_buf_len) {
    char *pstr = string_buf;
    size_t i, j;

    j = 0;

    for (i = 0; i < num_buf_len; i++) {
        int pn_digit = num_buf[i] >> 4;

        *pstr++ = pn_lut[pn_digit];
        j++;

        if (j >= (string_buf_len - 1)) break;

        pn_digit = num_buf[i] & 0x0f;
        *pstr++ = pn_lut[pn_digit];
        j++;
        if (j >= (string_buf_len - 1)) break;
    }

    *pstr = '\0';
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

char *call_type_to_string(uint8_t call_type, char *string_buf, size_t string_buf_len) {
    size_t len_call_type, len_pmt_type;

    len_call_type = strlen(call_type_str[call_type & 0x0f]);
    len_pmt_type = strlen(pmt_type_str[call_type >> 4]);

    if ((len_call_type + len_pmt_type + 1) > string_buf_len) {
        return NULL;
    }

    snprintf(string_buf, string_buf_len, "%s %s",
        call_type_str[call_type & 0x0f],
        pmt_type_str[call_type >> 4]);

    return string_buf;
}

char *timestamp_to_string(uint8_t *timestamp, char *string_buf, size_t string_buf_len) {
    snprintf(string_buf, string_buf_len, "%04d-%02d-%02d %02d:%02d:%02d",
        timestamp[0] + 1900,
        timestamp[1],
        timestamp[2],
        timestamp[3],
        timestamp[4],
        timestamp[5]);

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
    "0x16",                         // 0x16
    "0x17",                         // 0x17
    "0x18",                         // 0x18
    "0x19",                         // 0x19
    "DLOG_MT_FCONFIG_OPTS",         // 0x1a
    "0x1b",                         // 0x1b
    "0x1c",                         // 0x1c
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
    "0x27",                         // 0x27
    "0x28",                         // 0x28
    "0x29",                         // 0x29
    "DLOG_MT_ATN_REQ_CDR_UPL",      // 0x2a
    "0x2b",                         // 0x2b
    "DLOG_MT_ATN_REQ_TAB_UPD",      // 0x2c
    "0x2d",                         // 0x2d
    "0x2e",                         // 0x2e
    "0x2f",                         // 0x2f
    "0x30",                         // 0x30
    "0x31",                         // 0x31
    "DLOG_MT_COIN_VAL_TABLE",       // 0x32
    "DLOG_MT_CASH_BOX_COLLECTION",  // 0x33
    "0x34",                         // 0x34
    "DLOG_MT_CALL_DETAILS",         // 0x35
    "0x36",                         // 0x36
    "DLOG_MT_REP_DIAL_LIST",        // 0x37
    "DLOG_MT_SUMMARY_CALL_STATS",   // 0x38
    "DLOG_MT_CARRIER_CALL_STATS",   // 0x39
    "DLOG_MT_LIMSERV_DATA",         // 0x3a
    "0x3b",                         // 0x3b
    "DLOG_MT_SW_VERSION",           // 0x3c
    "0x3d",                         // 0x3d
    "DLOG_MT_NUM_PLAN_TABLE",       // 0x3e
    "DLOG_MT_RATE_REQUEST",         // 0x3f
    "DLOG_MT_RATE_RESPONSE",        // 0x40
    "DLOG_MT_AUTH_RESP_CODE",       // 0x41
    "0x42",                         // 0x42
    "0x43",                         // 0x43
    "0x44",                         // 0x44
    "0x45",                         // 0x45
    "0x46",                         // 0x46
    "DLOG_MT_CARRIER_STATS_EXP",    // 0x47
    "DLOG_MT_SPARE_TABLE",          // 0x48
    "DLOG_MT_RATE_TABLE",           // 0x49
    "0x4a",                         // 0x4a
    "0x4b",                         // 0x4b
    "0x4c",                         // 0x4c
    "0x4d",                         // 0x4d
    "0x4e",                         // 0x4e
    "0x4f",                         // 0x4f
    "0x50",                         // 0x50
    "0x51",                         // 0x51
    "0x52",                         // 0x52
    "0x53",                         // 0x53
    "0x54",                         // 0x54
    "DLOG_MT_VIS_PROPTS_L1",        // 0x55
    "DLOG_MT_VIS_PROPTS_L2",        // 0x56
    "0x57",                         // 0x57
    "0x58",                         // 0x58
    "0x59",                         // 0x59
    "0x5a",                         // 0x5a
    "0x5b",                         // 0x5b
    "DLOG_MT_CALL_SCREEN_LIST",     // 0x5c
    "DLOG_MT_SCARD_PARM_TABLE",     // 0x5d
    "0x5e",                         // 0x5e
    "0x5f",                         // 0x5f
    "0x60",                         // 0x60
    "0x61",                         // 0x61
    "0x62",                         // 0x62
    "0x63",                         // 0x63
    "0x64",                         // 0x64
    "0x65",                         // 0x65
    "0x66",                         // 0x66
    "0x67",                         // 0x67
    "0x68",                         // 0x68
    "0x69",                         // 0x69
    "0x6a",                         // 0x6a
    "0x6b",                         // 0x6b
    "0x6c",                         // 0x6c
    "0x6d",                         // 0x6d
    "0x6e",                         // 0x6e
    "0x6f",                         // 0x6f
    "0x70",                         // 0x70
    "0x71",                         // 0x71
    "0x72",                         // 0x72
    "0x73",                         // 0x73
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
    "DLOG_MT_CARD_TABLE",           // 0x86
    "DLOG_MT_CARRIER_TABLE",        // 0x87
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
    "0x98",                         // 0x98
    "0x99",                         // 0x99
    "0x9a",                         // 0x9a
    "0x9b",                         // 0x9b
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

const char *table_to_string(uint8_t table) {
    if (table >= (sizeof(table_string) / sizeof(char *))) {
        table = 0;
    }

    return (table_string[table]);
}
