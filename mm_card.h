/*
 * CARD Table 134 (0x86) data structures and definitions.
 *
 * See CARD (Expanded Card table) pp. 2-51
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020, Howard M. Harte
 *
 */

#ifndef MM_CARD_H_
#define MM_CARD_H_

#include <stdint.h>

#pragma pack(push)
#pragma pack(1)                 /* Pack data structures for communication with terminal. */

#define CCARD_MAX           32  /* Number of entries in CARD table MTR 2.x */
#define CCARD_MAX_MTR1      20  /* Number of entries in CARD table MTR 1.x */
#define SERVICE_CODE_LEN    20
#define SVC_CODE_MAX        5
#define SPILL_STRING_LEN    8
#define SC_CHECK_DIGIT_LEN  6
#define SC_CHECK_VALUE_LEN  8
#define SC_MANUF_LEN        5

typedef struct service_code_cc {
    uint16_t svc_code[SVC_CODE_MAX];
    uint8_t  spill_string[SPILL_STRING_LEN];
    uint8_t  term_char;
    uint8_t  discount_index;
} service_code_cc_t;

typedef struct service_code_sc {
    uint8_t check_digits[SC_CHECK_DIGIT_LEN];
    uint8_t check_value[SC_CHECK_VALUE_LEN];
    uint8_t manufacturer[SC_MANUF_LEN];
    uint8_t discount_index;
} service_code_sc_t;

typedef union service_code {
    uint8_t           raw[SERVICE_CODE_LEN];
    service_code_cc_t cc;
    service_code_sc_t sc;
} service_code_t;

/* CARD (Expanded Card table, MTR 2.x) pp. 2-57 */
typedef struct card_entry {
    uint8_t        pan_start[3];  /* Credit Card PAN-range start. */
    uint8_t        pan_end[3];    /* Credit Card PAN-range end. */
    uint8_t        standard_cd;
    uint8_t        vfy_flags;
    uint8_t        p_exp_date;
    uint8_t        p_init_date;
    uint8_t        p_disc_data;
    service_code_t svc_code;
    uint8_t        ref_num;
    uint8_t        carrier_ref;   /* Unique number for each carrier used to cross reference the carrier in other tables. */
    uint8_t        control_info;  /* Control-info 1 */
    uint8_t        bank_info;
    uint8_t        lang_code;
} card_entry_t;

typedef struct dlog_mt_card_table {
    card_entry_t c[CCARD_MAX];  /* 32 entries */
} dlog_mt_card_table_t;

/* CARD (Card table, MTR 1.x) */
typedef struct card_entry_mtr1 {
    uint8_t        pan_start[3];  /* Credit Card PAN-range start. */
    uint8_t        pan_end[3];    /* Credit Card PAN-range end. */
    uint8_t        standard_cd;
    uint8_t        vfy_flags;
    uint8_t        p_exp_date;
    uint8_t        p_init_date;
    uint8_t        p_disc_data;
    service_code_t svc_code;
    uint8_t        ref_num;
    uint8_t        carrier_ref;   /* Unique number for each carrier used to cross reference the carrier in other tables. */
} card_entry_mtr1_t;

typedef struct dlog_mt_card_table_mtr1 {
    card_entry_mtr1_t c[CCARD_MAX_MTR1];  /* 20 entries */
} dlog_mt_card_table_mtr1_t;

typedef enum std_card_type {
    undefined = 0,
    mod10,
    ansi,
    aba,
    cba,
    boc,
    ansi59,
    ccitt,
    pinoff,
    hello,
    smcard,
    reserved,
    scgpm416,
    sc_pcos,
    sc_mpcos,
    proton
} std_card_type_t;

/* See CARD (Expanded Card table) pp. 2-51 */
/* Note: These bit definitions are split in the docs. */
#define CARD_VF_MOD10_IND                   (1 << 0)  // Indicates whether a Mod 10 check should be performed on a Card Number when
                                                      // it is being added to the Hotlist table.
#define CARD_VF_NCCVAL_IND                  (1 << 1)  // Indicates whether validation must be done by the Millennium Manager before
                                                      // the terminal will permit a call to be made.
#define CARD_VF_CALLING_CARD_IND            (1 << 2)  // Indicates whether this type of card is *NOT* a calling card. “0” Yes -
                                                      // Calling Card, “1” No - Not a Calling Card.
#define CARD_VF_IMMEDIATE_AUTH_IND          (1 << 3)  // Indicates when the terminal should perform a card authorization. “0” Wait
                                                      // until called number obtained, “1” Request authorization immediately after
                                                      // card withdrawn.
#define CARD_VF_SERVICE_CD_VALIDATION_IND   (1 << 4)  // Indicates whether the list of Card Service Codes should be checked using
                                                      // positive or negative validation. “0” Negative, None; “1” Positive
#define CARD_VF_PROMPT_FOR_PIN              (1 << 5)  // Indicates whether to prompt for the PIN number or not.
#define CARD_VF_PROMPT_FOR_TELCO_PIN        (1 << 6)  // Indicates whether to prompt for Telco PIN or not.
#define CARD_VF_ACCS_ROUTING                (1 << 7)  // “0” Route to Millennium Manager, “1” Route to ACCS

#pragma pack(pop)

#endif  // MM_CARD_H_
