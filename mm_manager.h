/*
 * This is a "Manager" for the Nortel Millennium payhone.
 *
 * It can provision a Nortel Millennium payphone with Rev 1.0 or 1.3
 * Control PCP.  CDRs, Alarms, and Maintenance Reports can also be
 * retieved.
 *
 * www.github.com/hharte/mm_manager
 *
 * (c) 2020, Howard M. Harte
 */

#define RX 2
#define TX 3

#define START_BYTE      (0x02)
#define STOP_BYTE       (0x03)

#define FLAG_DISCONNECT (1 << 5)
#define FLAG_ACK        (1 << 3)
#define FLAG_RETRY      (1 << 2)
#define FLAG_SEQUENCE   (0x3)

#define PKT_SUCCESS                 (0)
#define PKT_ERROR_RETRY             (1 << 0)
#define PKT_ERROR_DISCONNECT        (1 << 1)
#define PKT_ERROR_CRC               (1 << 2)
#define PKT_ERROR_FRAMING           (1 << 3)
#define PKT_ERROR_TIMEOUT           (1 << 4)
#define PKT_ERROR_EOF               (1 << 5)

#define PKT_TIMEOUT_MAX             (10)    // Maximum time  to wait for modem character

#define PKT_TABLE_ID_OFFSET         (0x05)
#define PKT_TABLE_DATA_OFFSET       (PKT_TABLE_ID_OFFSET + 1)

#define PKT_TABLE_DATA_LEN_MAX      (245)   // Maximum table data length

/* TERMSCH (Terminal Schedule) pp. 2-533
 * Renamed per https://wiki.millennium.management/dlog:start
 */
#define DLOG_MT_CARD_AUTH_REQ       0x01    // 1: Card Authorization Request
#define DLOG_MT_FUNF_CARD_AUTH      0x02    // 2: Formatted/Unformatted Authorization Request – Post MSR 1.5 message.
#define DLOG_MT_AUTH_RESPONSE       0x03    // 3: Authorization Response
#define DLOG_MT_CD_CALL_DETAILS     0x04    // 4: Card Call Detail Record
#define DLOG_MT_CDR_DETAILS_ACK     0x05    // 5: Call Detail Acknowledge
#define DLOG_MT_MAINT_REQ           0x06    // 6: Maintenance Action Report/Request Message
#define DLOG_MT_ALARM               0x07    // 7: Alarm Message
#define DLOG_MT_CALL_IN             0x08    // 8: Terminal Call-in Message
#define DLOG_MT_CALL_BACK           0x09    // 9: Terminal Call-back Message
#define DLOG_MT_TERM_STATUS         0x0a    // 10: Terminal Status Message
#define DLOG_MT_CD_CALL_STATS       0x0b    // 11: Summary Card Call Statistics Record
#define DLOG_MT_PERF_STATS          0x0c    // 12: Performance Statistics Record
#define DLOG_MT_END_DATA            0x0d    // 13: End of Data Message
#define DLOG_MT_TAB_UPD_ACK         0x0e    // 14: Table Update Acknowledge Message
#define DLOG_MT_MAINT_ACK           0x0f    // 15: Maintenance Acknowledge
#define DLOG_MT_ALARM_ACK           0x10    // 16: Alarm Acknowledge
#define DLOG_MT_TRANS_DATA          0x11    // 17: Transmit Terminal Data
#define DLOG_MT_TABLE_UPD           0x12    // 18: Terminal Table Data Update
#define DLOG_MT_CALL_BACK_REQ       0x13    // 19: Call Back Request
#define DLOG_MT_TIME_SYNC           0x14    // 20: Date/Time Synchronization
#define DLOG_MT_NCC_TERM_PARAMS     0x15    // 21: Terminal Access Parameters
#define DLOG_MT_FCONFIG_OPTS        0x1a    // 26: Feature Configuration – Universal
#define DLOG_MT_ADVERT_PROMPTS      0x1d    // 29: Advertising prompts
#define DLOG_MT_USER_IF_PARMS       0x1e    // 30: User Interface Parameters
#define DLOG_MT_INSTALL_PARAMS      0x1f    // 31: Installation/Servicing Parameters
#define DLOG_MT_COMM_STAT_PARMS     0x20    // 32: Communication Statistics and Configuration Parameters
#define DLOG_MT_MODEM_PARMS         0x21    // 33: Modem Configuration Parameters
#define DLOG_MT_CALL_STAT_PARMS     0x22    // 34: Call / Carrier Statistics Parameters
#define DLOG_MT_CALL_IN_PARMS       0x23    // 35: Time/Call-In Parameters
#define DLOG_MT_TIME_SYNC_REQ       0x24    // 36: Time Synchronization Request
#define DLOG_MT_PERF_STATS_MSG      0x25    // 37: Performance Statistics Record Message
#define DLOG_MT_CASH_BOX_STATUS     0x26    // 38: Cash Box Status Message – Universal
#define DLOG_MT_ATN_REQ_CDR_UPL     0x2a    // 42: Attention Request Call Records Upload
#define DLOG_MT_ATN_REQ_TAB_UPD     0x2c    // 44: Attention Request Table Update
#define DLOG_MT_COIN_VAL_TABLE      0x32    // 50: Coin Validation Table
#define DLOG_MT_CASH_BOX_COLLECTION 0x33    // 51: Cash Box Collection Message – Universal
#define DLOG_MT_CALL_DETAILS        0x35    // 53: Call Detail Record Message – Post MSR 1.5
#define DLOG_MT_REP_DIAL_LIST       0x37    // 55: Enhanced Repertory Dialer List
#define DLOG_MT_SUMMARY_CALL_STATS  0x38    // 56: Summary Call Statistics Record – Post MSR 1.6
#define DLOG_MT_CARRIER_CALL_STATS  0x39    // 57: Carrier Call Statistics Message
#define DLOG_MT_LIMSERV_DATA        0x3a    // 58: (Limit) Service Level Table
#define DLOG_MT_SW_VERSION          0x3c    // 60: Terminal Software Version Message
#define DLOG_MT_NUM_PLAN_TABLE      0x3e    // 62: Numbering Plan Table
#define DLOG_MT_RATE_REQUEST        0x3f    // 63: Rate Request Message – Universal
#define DLOG_MT_RATE_RESPONSE       0x40    // 64: Rate Response Message – Universal
#define DLOG_MT_AUTH_RESP_CODE      0x41    // 65: Authorization Response Message – Post MSR 1.5
#define DLOG_MT_CARRIER_STATS_EXP   0x47    // 71: Expanded Carrier Call Statistics Message
#define DLOG_MT_SPARE_TABLE         0x48    // 72: Spare Table
#define DLOG_MT_RATE_TABLE          0x49    // 73: Rate Table
#define DLOG_MT_VIS_PROPTS_L1       0x55    // 85: Expanded Visual Prompts Language A
#define DLOG_MT_VIS_PROPTS_L2       0x56    // 86: Expanded Visual Prompts Language B
#define DLOG_MT_CALL_SCREEN_LIST    0x5c    // 92B: 180 Number Call Screening List
#define DLOG_MT_SCARD_PARM_TABLE    0x5d    // 93: Smart Card Parameters Table
#define DLOG_MT_CARD_TABLE          0x86    // 134: Expanded Card Table (32 Entries)
#define DLOG_MT_CARRIER_TABLE       0x87    // 135: Expanded Carrier Table (33 Entries)
#define DLOG_MT_NPA_NXX_TABLE_1     0x88    // 136: Double Compressed LCD Table 1
#define DLOG_MT_NPA_NXX_TABLE_2     0x89    // 137: Double Compressed LCD Table 2
#define DLOG_MT_NPA_NXX_TABLE_3     0x8a    // 138: Double Compressed LCD Table 3
#define DLOG_MT_NPA_NXX_TABLE_4     0x8b    // 139: Double Compressed LCD Table 4
#define DLOG_MT_NPA_NXX_TABLE_5     0x8c    // 140: Double Compressed LCD Table 5
#define DLOG_MT_NPA_NXX_TABLE_6     0x8d    // 141: Double Compressed LCD Table 6
#define DLOG_MT_NPA_NXX_TABLE_7     0x8e    // 142: Double Compressed LCD Table 7
#define DLOG_MT_NPA_NXX_TABLE_8     0x8f    // 143: Double Compressed LCD Table 8
#define DLOG_MT_NPA_NXX_TABLE_9     0x90    // 144: Double Compressed LCD Table 9
#define DLOG_MT_NPA_NXX_TABLE_10    0x91    // 145: Double Compressed LCD Table 10
#define DLOG_MT_NPA_NXX_TABLE_11    0x92    // 146: Double Compressed LCD Table 11
#define DLOG_MT_NPA_NXX_TABLE_12    0x93    // 147: Double Compressed LCD Table 12
#define DLOG_MT_NPA_NXX_TABLE_13    0x94    // 148: Double Compressed LCD Table 13
#define DLOG_MT_NPA_NXX_TABLE_14    0x95    // 149: Double Compressed LCD Table 14
#define DLOG_MT_NPA_NXX_TABLE_15    0xa0    // 154: Double Compressed LCD Table 15
#define DLOG_MT_NPA_NXX_TABLE_16    0xa1    // 155: Double Compressed LCD Table 16
#define DLOG_MT_NPA_SBR_TABLE       0x96    // 150: Set Based Rating NPA Table
#define DLOG_MT_INTL_SBR_TABLE      0x97    // 151: Set Based Rating International Table

/* TTBLREQ (Terminal Table Request) pp. 2-651 */
#define TTBLREQ_CRAFT_FORCE_DL      0x01    // Menu Item - Craft I/F Table Download
#define TTBLREQ_CRAFT_INSTALL       0x02    // Install via Craft I/F
#define TTBLREQ_LOST_MEMORY         0x04    // Lost Memory
#define TTBLREQ_PWR_LOST_ON_DL      0x08    // Power Lost on Download
#define TTBLREQ_CASHBOX_STATUS      0x80    // Cash Box Status Requested

/* TCOLLCT (Terminal Cash Box Collections) pp. 2-464 */
#define CASHBOX_STATUS_NORMAL           0x00    // Normal State
#define CASHBOX_STATUS_VALUE_EXCEEDED   0x01    // Dollar Value exceeded
#define CASHBOX_STATUS_THRESH_EXCEEDED  0x02    // Percent Threshold exceeded
#define CASHBOX_STATUS_BOTH_EXCEEDED    0x03    // Both dollar value and percent threshold exceeded
#define CASHBOX_STATUS_FULL             0x04    // Cashbox totally full

#pragma pack(push)
#pragma pack(1)         /* Pack data structures for communication with terminal. */

/* Structure of the Millennium Manager Packet Header */
typedef struct mm_packet_header {
    uint8_t start;      /* Start byte (0x02) */
    uint8_t flags;      /* Flags and sequence number */
    uint8_t pktlen;     /* Packet length, from flags to end byte. */
} mm_packet_header_t;

typedef struct mm_packet_trailer {
    uint16_t crc;       /* CRC-16 including start byte through payload. */
    uint8_t end;        /* End byte (0x03) */
} mm_packet_trailer_t;

typedef struct mm_packet {
    mm_packet_header_t  hdr;        /* MM packet header */
    uint8_t payload[250];           /* Data payload */
    mm_packet_trailer_t trailer;    /* MM packet trailer */
    uint8_t payload_len;            /* Metadata not part of actual packet on the wire. */
    uint16_t calculated_crc;        /* Metadata not part of actual packet on the wire. */
} mm_packet_t;

typedef struct mm_table {
    uint8_t table_id;
    uint8_t data[10 * 1024];
    mm_packet_t pkt;
} mm_table_t;

/*
 * These data structures match the ones generated and
 * consumed by the terminal.
 */
typedef struct dlog_mt_alarm {
    uint8_t timestamp[6];
    uint8_t alarm_id;
} dlog_mt_alarm_t;

typedef struct dlog_mt_maint_req {
    uint16_t type;
    uint8_t access_pin[3];
} dlog_mt_maint_req_t;

typedef struct dlog_mt_call_back_req {
    uint16_t type;
    uint8_t callback_time[6];
} dlog_mt_call_back_req_t;

typedef struct dlog_mt_ncc_term_params {
    uint8_t terminal_id[5];
    uint8_t pri_ncc_number[10];
    uint8_t sec_ncc_number[10];
    uint8_t unknown[22];
} dlog_mt_ncc_term_params_t;

/* From TERM table, pp. 2-517 */
typedef struct dlog_mt_call_in_params {
    uint8_t call_in_start_date[3];      /* Call-in start date YY:MM:DD */
    uint8_t call_in_start_time[3];      /* Call-in start date HH:MM:SS */
    uint8_t call_in_interval[3];        /* Call-in inteval DD:HH:MM */
    uint8_t call_back_retry_time[2];    /* Call-back retry time MM:SS */
    uint8_t cdr_threshold;              /* Indicates the number of CDRs that the terminal will store before automatically calling in to the Millennium Manager to upload them. (Range: 1-50) */
    uint8_t unknown_timestamp[6];       /* Looks like a timestamp, but not sure what it's for. */
    uint8_t unknown[2];
} dlog_mt_call_in_params_t;

/* From CALLST (Call Statistics Parameters) pp. 2-36 */
typedef struct dlog_mt_call_stat_params {
    uint8_t callstats_start_time[2];    /* Call stats start date HH:MM */
    uint8_t callstats_duration;         /* Indicates the number of days over which call statistics will be accumulated. */
    uint8_t callstats_threshold;        /* Indicates the number of Call Statistics records that the terminal will store before automatically calling in to the Millennium Manager to upload them. */
    uint8_t timestamp[4][2];            /* These timestamps, entered in the terminal's local time, are used with the Call Statistics Start Time to partition the 24-hour recording period into as many as 5 time periods. */
    uint8_t enable;                     /* 0=No (Disabled), 1=Yes (Enabled), 2=Summarized Carrier Statistics */
    uint8_t cdr_threshold;              /* Indicates the number of Full CDR Logging records that the terminal will store before automatically calling in to the Millennium Manager to upload them. */
    uint8_t cdr_start_time[2];          /* HH:MM Indicates the time of day the terminal should begin full CDR logging, in the terminal local time. */
    uint8_t cdr_duration_days;          /* Indicates the number of days over which to perform Full CDR Logging. */
    uint8_t cdr_duration_hours_flags;   /* 0-23, Bit 6: Indicates whether statistics should be recorded for all complete calls except local coin. */
} dlog_mt_call_stat_params_t;

/* From COMMST (Communications Statistics Parameters) pp. 2-86 */
typedef struct dlog_mt_comm_stat_params {
    uint16_t co_access_dial_complete;       /* 100 */
    uint16_t co_access_dial_complete_int;   /* 50 */
    uint16_t dial_complete_carr_detect;     /* 100 */
    uint16_t dial_complete_carr_detect_int; /* 50 */
    uint16_t pad[2];
    uint16_t carr_detect_first_pac;         /* 10 */
    uint16_t carr_detect_first_pac_int;     /* 5 */
    uint16_t user_waiting_expect_info;      /* 600 */
    uint16_t user_waiting_expect_info_int;  /* 100 */
    uint8_t perfstats_threshold;            /* 2 Indicates the number of Perf Statistics records that the terminal will store before automatically calling in to the Millennium Manager to upload them. */
    uint8_t perfstats_start_time[2];        /* Perf stats start date HH:MM */
    uint8_t perfstats_duration;             /* Indicates the number of days over which Perf statistics will be accumulated. */
    uint8_t perfstats_timestamp[4][2];      /* These timestamps, entered in the terminal's local time, are used with the Perf Statistics Start Time to partition the 24-hour recording period into as many as 5 time periods. */
} dlog_mt_comm_stat_params_t;

/* From USERPRM (User Interface Parameters) pp. 2-670 */
typedef struct dlog_mt_user_if_params {
    uint16_t digit_clear_delay;             /* 450 */
    uint16_t transient_delay;               /* 450 */
    uint16_t transient_hint_time;           /* 450 */
    uint16_t visual_to_voice_delay;         /* 450 */
    uint16_t voice_repitition_delay;        /* 450 */
    uint16_t no_action_timeout;             /* 3000 (30s) */
    uint16_t card_validation_timeout;       /* 3000 (30s) */
    uint16_t dj_second_string_dtmf_timeout; /* 300 (3s) */
    uint16_t spare_timer_b;                 /* 1100 (11s) */
    uint16_t cp_input_timeout;              /* 100 (1s) */
    uint16_t language_timeout;              /* 1000 (10s) */
    uint16_t cfs_timeout;                   /* 200 (2s) */
    uint16_t called_party_disconnect;       /* 1200 (12s) */
    uint8_t  no_voice_prompt_reps;          /* 3 */
    uint16_t accs_digit_timeout;            /* 450 (4.5s) */
    uint16_t collect_call_timeout;          /* 400 (4s) */
    uint16_t bong_tone_timeout;             /* 1600 (16s) */
    uint16_t accs_no_action_timeout;        /* 450 (4.5s) */
    uint16_t card_auth_required_timeout;    /* 6000 (60s) */
    uint16_t rate_request_timeout;          /* 6000 (60s) */
    uint16_t manual_dial_hold_time;         /* 50 (0.5s) */
    uint16_t autodialer_hold_time;          /* 300 (3s) */
    uint16_t coin_first_warning_time;       /* 30 (30s) */
    uint16_t coin_second_warning_time;      /* 5 (5s) */
    uint16_t alternate_bong_tone_timeout;   /* 1000 (10s) */
    uint16_t delay_after_bong_tone;         /* 200 (2s) */
    uint16_t alternate_delay_after_bong_tone; /* 150 (1.5s) */
    uint16_t display_scroll_speed;          /* 10 (100ms) */
    uint16_t spare_timer_c;                 /* 0 */
    uint16_t aos_bong_tone_timeout;         /* 600 (6s) */
    uint16_t fgb_aos_second_spill_timeout;  /* 200 (2s) */
    uint16_t datajack_connect_timeout;      /* 15000 (150s) */
    uint16_t datajack_pause_threshold;      /* 45 (450ms) */
    uint16_t datajack_ias_timer;            /* 10 (100ms) */
} dlog_mt_user_if_params_t;

typedef struct dlog_mt_call_details {
    uint8_t rate_type;
    uint8_t called_num[10];
    uint8_t carrier_code;
    uint8_t card_num[10];
    uint32_t call_cost[2];
    uint16_t seq;
    uint8_t start_timestamp[6];
    uint8_t call_duration[3];
    uint8_t call_type;              /* CALLTYP (Call Type) pp. 2-41 */
    uint8_t pad[11];
} dlog_mt_call_details_t;

/* Call Type (lower 4-bits) of CALLTYP */
#define CALL_TYPE_INCOMING      0x00    // Incoming
#define CALL_TYPE_UNANSWERED    0x01    // Unanswered
#define CALL_TYPE_ABANDONED     0x02    // Abandoned
#define CALL_TYPE_LOCAL         0x03    // Local
#define CALL_TYPE_INTRA_LATA    0x04    // Intra-LATA
#define CALL_TYPE_INTER_LATA    0x05    // Inter-LATA
#define CALL_TYPE_INTERNATIONAL 0x06    // International
#define CALL_TYPE_OPERATOR      0x07    // Operator
#define CALL_TYPE_ZERO_PLUS     0x08    // Zero+
#define CALL_TYPE_1800          0x09    // 1-800
#define CALL_TYPE_DIR_ASSIST    0x0a    // Directory Assistance
#define CALL_TYPE_DENIED        0x0b    // Denied
#define CALL_TYPE_UNASSIGNED    0x0c    // Unassigned
#define CALL_TYPE_UNASSIGNED2   0x0d    // Unassigned
#define CALL_TYPE_E_PURSE       0x0e    // e-Purse transaction
#define CALL_TYPE_UNKNOWN       0x0f    // Unknown

/* Payment Type (upper 4-bits) of CALLTYP */
#define PMT_TYPE_UNUSED         0x00    // Unused for compatibility reasons
#define PMT_TYPE_UNUSED2        0x01    // Unused for compatibility reasons
#define PMT_TYPE_NONE           0x02    // operator, bill to third, free, etc.
#define PMT_TYPE_COIN           0x03    // Coin
#define PMT_TYPE_CREDIT_CARD    0x04    // Commercial credit card (magnetic)
#define PMT_TYPE_CALLING_CARD   0x05    // Calling card (magnetic)
#define PMT_TYPE_CASH_CARD      0x06    // Cash card
#define PMT_TYPE_INMATE         0x07    // Inmate no payment
#define PMT_TYPE_MONDEX         0x08    // Mondex card
#define PMT_TYPE_VISA_ST_VALUE  0x09    // Visa stored value card
#define PMT_TYPE_SMART_CITY     0x0a    // Smart City (FSU)
#define PMT_TYPE_PROTON         0x0b    // Proton
#define PMT_TYPE_UNDEFINED      0x0c    // Undefined
#define PMT_TYPE_UNDEFINED2     0x0d    // Undefined
#define PMT_TYPE_UNDEFINED3     0x0e    // Undefined
#define PMT_TYPE_UNDEFINED4     0x0f    // Undefined

/* DLOG_MT_CASH_BOX_COLLECTION */
typedef struct dlog_mt_cash_box_collection {
    uint8_t pad[14];
    uint8_t timestamp[6];
    uint8_t pad2[4];
    uint8_t status;                     // Cash box status bits (0=normal, 1=$value exceeded, 2=%threshold exceeded, 3=both exceeded, >3 totally full)
    uint8_t percent_full;               // Percent full (0-100%)
    uint16_t currency_value;            // Contains the total value of the currency which was collected, not including any previous collections.
    uint16_t pad3[2];
    uint16_t coin_count[8];             // Array of counts of Nickles, Dime, Quarters, Dollars for US and CA.
    uint8_t spare[22];
} dlog_mt_cash_box_collection_t;

#define COIN_COUNT_CA_NICKELS   0
#define COIN_COUNT_CA_DIMES     1
#define COIN_COUNT_CA_QUARTERS  2
#define COIN_COUNT_CA_DOLLARS   3
#define COIN_COUNT_US_NICKELS   4
#define COIN_COUNT_US_DIMES     5
#define COIN_COUNT_US_QUARTERS  6
#define COIN_COUNT_US_DOLLARS   7

/* TABLE_ID_CASHBOX_STATUS_UNIV */
typedef struct cashbox_status_univ {
    uint8_t timestamp[6];
    uint8_t pad[4];
    uint8_t status;                     // Cash box status bits (0=normal, 1=$value exceeded, 2=%threshold exceeded, 3=both exceeded, >3 totally full)
    uint8_t percent_full;               // Percent full (0-100%)
    uint16_t currency_value;            // Contains the total value of the currency which was collected, not including any previous collections.
    uint16_t pad2[2];
    uint16_t coin_count[8];             // Array of counts of Nickles, Dime, Quarters, Dollars for US and CA.
    uint8_t spare[22];
} cashbox_status_univ_t;

/* DLOG_MT_PERF_STATS_RECORD 97 bytes */
typedef struct dlog_mt_perf_stats_record {
    uint8_t timestamp[6];
    uint8_t timestamp2[6];
    uint16_t stats[43];
} dlog_mt_perf_stats_record_t;

/* DLOG_MT_SUMMARY_CALL_STATS - TCALSTE (Terminal Call Statistics Enhanced) pp. 2-393 */
typedef struct dlog_mt_summary_call_stats {
    uint8_t timestamp[6];                   /* Summary period start timestamp. */
    uint8_t timestamp2[6];                  /* Summary period end timestamp. */
    uint16_t stats[16];                     /* Call counts for different types of calls. */
    uint16_t rep_dialer_peg_count[10];      /* Counts for each of the reperatory dialer keys. */
    uint32_t total_call_duration;           /* Total call duration (seconds,) timed from answer supervision to on-hook. */
    uint32_t total_time_off_hook;           /* The total amount of time (seconds) the receiver was off-hook. */
    uint16_t free_featb_call_count;         /* The number of Feature Group B calls. */
    uint16_t datajack_calls_attempt_count;  /* Number of datajack calls attempted. */
    uint16_t completed_1800_billable_count; /* Number of completed 1-800 calls that were billable. */
    uint16_t datajack_calls_complete_count; /* Datajack calls that were completed. */
} dlog_mt_summary_call_stats_t;

typedef struct carrier_stats_entry {
    uint8_t carrier_ref;
    uint16_t stats[29];
} carrier_stats_entry_t;

/* DLOG_MT_CARRIER_CALL_STATS 106 bytes */
typedef struct dlog_mt_carrier_call_stats {
    uint8_t timestamp[6];
    uint8_t timestamp2[6];
    carrier_stats_entry_t carrier_stats[3];
} dlog_mt_carrier_call_stats_t;

/* See TCARRST (Terminal Carrier Call Statistics) pp. 2-406 */
#define STATS_EXP_CALL_TYPE_MAX     4
#define STATS_EXP_PAYMENT_TYPE_MAX  12

typedef struct carrier_stats_exp_entry {
    uint8_t carrier_ref;
    uint16_t stats[4][12];                  /* 4 call types: local, Intra-LATA, Inter-LATA, IXL.  12 stats each. */
    uint16_t operator_assist_call_count;
    uint16_t zero_plus_call_count;
    uint16_t free_featb_call_count;
    uint16_t directory_assist_call_count;
    uint32_t total_call_duration;           /* Total call duration (seconds) */
    uint16_t total_insert_mode_calls;
    uint16_t total_manual_mode_calls;
    uint16_t spare_counter;
} carrier_stats_exp_entry_t;

/* DLOG_MT_CARRIER_STATS_EXP TCARRST (Terminal Carrier Call Statistics) pp. 2-406 */
#define CARRIER_STATS_EXP_MAX_CARRIERS  2
typedef struct dlog_mt_carrier_stats_exp {
    uint8_t timestamp[6];
    uint8_t timestamp2[6];
    uint8_t stats_vintage;
    carrier_stats_exp_entry_t carrier[CARRIER_STATS_EXP_MAX_CARRIERS];
} dlog_mt_carrier_stats_exp_t;

/* DLOG_MT_SW_VERSION (TSWVERS pp. 2-647) */
typedef struct dlog_mt_sw_version {
    uint8_t control_rom_edition[7];
    uint8_t control_version[4];
    uint8_t telephony_rom_edition[7];
    uint8_t telephony_version[4];
    uint8_t term_type;
    uint8_t validator_sw_ver[2];
    uint8_t validator_hw_ver[2];
} dlog_mt_sw_version_t;

/* Constants and data structures for the RATEINT (Set Based Rating - International) pp. 2-326
 *
 * Thanks to æstrid for figuring out the data structures for the International
 * Set-based Rating table, which is not documented in the Database Design
 * Report MSR 2.1.
 */

/* Convert International Rate Table flags to RATE table entry */
#define RATE_TABLE_OFFSET   28
#define IXL_TO_RATE(x)      ((x) + RATE_TABLE_OFFSET)

#define IXL_BLOCKED         1   // International blocked.
#define IXL_NCC_RATED       0   // International rate is determined by the NCC.

#define INTL_RATE_TABLE_MAX_ENTRIES  200
typedef struct intl_rate_table_entry {
    uint16_t ccode;
    uint8_t flags;          // 0 calls in the NCC to rate, 1 is "blocked", 2 - 63 are rate indices in the RATE table (add RATE_TABLE_OFFSET)
} intl_rate_table_entry_t;

typedef struct dlg_mt_intl_sbr_table {
    uint8_t flags;
    uint8_t default_rate_index; // 28 + default_rate_index is the entry in the RATE table.
    uint8_t spare;
    intl_rate_table_entry_t irate[INTL_RATE_TABLE_MAX_ENTRIES];
} dlg_mt_intl_sbr_table_t;

#define FLAG_PERIOD_UNLIMITED   (1 << 15)

typedef enum rate_type {
    mm_intra_lata = 0,      // 0 Millennium Manager rated Intra-lata
    lms_rate_local,         // 1 LMS rate (local call)
    fixed_charge_local,     // 2 Fixed Charge (local call)
    not_available,          // 3 Rate not available (Not used in RATE table)
    invalid_npa_nxx,        // 4 Invalid NPA/NXX
    toll_intra_lata,        // 5 Toll Intra-lata
    toll_inter_lata,        // 6 Toll Inter-lata
    mm_inter_lata,          // 7 Millennium Manager rated Inter-lata
    mm_local,               // 8 Millennium Manager rated local
    mm_international        // 9 International??
} rate_type_t;

typedef struct rate_table_entry {
    uint8_t  type;                          /* See CALLTYP (Call Type) pp. 2-41 */
    uint16_t initial_period;
    uint16_t initial_charge;
    uint16_t additional_period;
    uint16_t additional_charge;
} rate_table_entry_t;

#define RATE_TABLE_MAX_ENTRIES  128
typedef struct dlog_mt_rate_table {
    uint8_t unknown[39];
    rate_table_entry_t r[RATE_TABLE_MAX_ENTRIES];
} dlog_mt_rate_table_t;

/* DLOG_MT_RATE_REQUEST */
typedef struct dlog_mt_rate_request {
    uint16_t seq;
    uint8_t phone_number[10];
    uint8_t timestamp[6];
    uint8_t pad[6];
} dlog_mt_rate_request_t;

/* DLOG_MT_RATE_RESPONSE  (25 bytes) */
typedef struct dlog_mt_rate_response {
    rate_table_entry_t rate;
    uint8_t pad2[16];
} dlog_mt_rate_response_t;

/* DLOG_MT_FUNF_CARD_AUTH - see: TAUTH (Terminal Card Authorization) pp. 2-370 */
typedef struct dlog_mt_funf_card_auth {
    uint8_t control_flag;
    uint8_t phone_number[10];               /* Dialed number for which authorization is requested */
    uint8_t carrier_ref;                    /* Unique number for each carrier used to cross reference the carrier in other tables. */
    uint8_t card_number[12];                /* Card number, terminated with 0xe */
    uint16_t service_code;
    uint16_t unknown;
    uint8_t exp_yy;                         /* Card expiration year. */
    uint8_t exp_mm;                         /* Card expiration month. */
    uint16_t unknown2;
    uint8_t init_yy;                        /* Card initial year. */
    uint8_t init_mm;                        /* Card initial month. */
    uint8_t call_type;                      /* See CALLTYP (Call Type) pp. 2-41 */
    uint8_t card_ref_num;
    uint16_t seq;                           /* Authorization sequence number */
} dlog_mt_funf_card_auth_t;

/* DLOG_MT_AUTH_RESP_CODE */
typedef struct dlog_mt_auth_resp_code {
    uint8_t  resp_code;                     /* Authorization response code: 0=card valid, otherwise card invalid. */
    uint8_t  pad[20];
} dlog_mt_auth_resp_code_t;

typedef struct carrier_table_entry {
    uint8_t  carrier_ref;                   /* Unique number for each carrier used to cross reference the carrier in other tables. */
    uint16_t carrier_num;
    uint32_t valid_cards;
    char     display_prompt[20];            /* Actual prompt for display on one line. */
    uint8_t  control_byte2;
    uint8_t  control_byte;
    uint16_t fgb_timer;                     /* Time in 10 msec increments */
    uint8_t  international_accept_flags;
    uint8_t  call_entry;                    /* This field is used for Call Entry. This indicates the pointer to the Call Screening list for Feature Group B access. */
} carrier_table_entry_t;

#define DEFAULT_CARRIERS_MAX        9
#define CARRIER_TABLE_MAX_CARRIERS  33
typedef struct dlog_mt_carrier_table {
    uint8_t defaults[DEFAULT_CARRIERS_MAX];
    carrier_table_entry_t carrier[CARRIER_TABLE_MAX_CARRIERS];
    uint8_t spare[10];
} dlog_mt_carrier_table_t;

typedef struct rdlist_table_entry {
    uint8_t  pad[3];
    uint8_t  phone_number[8];
    char     display_prompt[40];
    uint8_t  pad2[6];
} rdlist_table_entry_t;

#define RDLIST_MAX                  10
typedef struct dlog_mt_rdlist_table {
    rdlist_table_entry_t rd[10];
} dlog_mt_rdlist_table_t;

/* CONTROL_BYTE2 - CARRIER (Carrier) pp. 2-71 */
#define CB2_FEATURE_GRP_B_PROMPT_FOR_NUM        (1 << 0)    /* FEATURE GRP B PROMPT FOR NUM */
#define CB2_REM_CARRIER_PREFIX_ZM_LOCAL         (1 << 1)    /* REM CARRIER PREFIX ZM LOCAL */
#define CB2_REM_CARRIER_PREFIX_INTRALATA        (1 << 2)    /* REM CARRIER PREFIX INTRALATA */
#define CB2_REM_CARRIER_PREFIX_INTERLATA        (1 << 3)    /* Remove Carrier Prefix Interlata */
#define CB2_REM_CARRIER_PREFIX_INTERNATIONAL    (1 << 4)    /* Remove Carrier Prefix International */
#define CB2_REM_CARRIER_PREFIX_DA               (1 << 5)    /* Remove Carrier Prefix DA */
#define CB2_REM_CARRIER_PREFIX_1800             (1 << 6)    /* Remove Carrier Prefix 1-800 */

/* CONTROL_BYTE - CARRIER (Carrier) pp. 2-72 */
#define CB_CARRIER_CARD_FLAG                    (1 << 0)    /* Indicates if to dial 101XXXX */
#define CB_USE_SPEC_DISPLAY_PROMPT              (1 << 1)    /* Indicates use of the specific display prompt. */
#define CB_ACCEPTS_COIN_CASH_CARDS              (1 << 2)    /* Indicates if coin calls are accepted. */
#define CB_USE_ALTERN_BONG_TONE_TIMEOUT         (1 << 3)    /* If this bit is 0, then the normal value from the User Interface Table will be used. If this bit is 1, then the alternate timer value from the User Interface Table will be used. */
#define CB_USE_ALTERN_DELAY_AFTER_BONG          (1 << 4)    /* If this bit is 0, then the normal value from the User Interface Table will be used. If this bit is 1, then the alternate timer value from the User Interface Table will be used. */
#define CB_INTRA_LATA_CALLS_TO_LEC              (1 << 5)    /* This field is used for intra-lata calls to LEC. This field determines how an intra-lata call is routed (to LEC or Carrier) when a carrier card is inserted and no carrier prefix is dialed. */
#define CB_OUTDIAL_STRING_ORDER                 (1 << 6)    /* 0=FGB#, Called #, Card #; 1=FGB#, Card #, Called # */
#define CB_FEATURE_GROUP_B                      (1 << 7)    /* Feature Group B is a category of free access to the carrier network. It usually uses 1-800 or 950xxx. */

/* CALLSCR (Call Screening List) pp. 2-33, and FREE (Free Call) pp. 2-187 */
typedef struct call_screen_list_entry {
    uint8_t  free_call_flags;               /* FREE (Free Call) pp. 2-189 */
    uint8_t  call_type;                     /* CALLTYPE */
    uint8_t  carrier_ref;                   /* Reference to the RATE/CARRIER table. A value of 255 means there is no reference. */
    uint8_t  ident2;                        /* IDENT2 Flags, see pp. 2-193 */
    uint8_t  phone_number[9];               /* 0-terminated phone number, one digit per nibble. F=single digit wildcard, B=link to another CALLSCR entry (in decimal.) */
    uint8_t  class;                         /* This seems to indicate class of number */
    uint8_t  spare[3];                      /* Reserved for future use. */
} call_screen_list_entry_t;

#define CALLSCRN_TABLE_MAX                      180
typedef struct dlog_mt_call_screen_list {
    call_screen_list_entry_t entry[CALLSCRN_TABLE_MAX];
} dlog_mt_call_screen_list_t;

#define CS_FREE_DENY_IND            (1 << 0)
#define CS_ANSWER_SUPERVISION       (1 << 1)
#define CS_KEYPAD_ENABLE_IND        (1 << 2)
#define CS_FAIL_TO_POTS_ONLY        (1 << 3)
#define CS_TRANSMITTER_MUTED        (1 << 4)
#define CS_TIMEOUT_BEFORE_DIALING   (1 << 5)
#define CS_DENY_CARD_PYMNT_IND      (1 << 6)
#define CS_FEATURE_GROUP_B_NUMBER   (1 << 7)

/* IDENT2 Flags, see pp. 2-193 */
#define CS_IDENT2_DENY_MDS          (1 << 0)    // Deny to Message Delivery Service (MDS)
#define CS_IDENT2_DENY_DJ           (1 << 1)    // Indicates if a call is deny to Datajack
#define CS_IDENT2_ALW_RES_SVC       (1 << 2)    // Allow in restricted service
#define CS_IDENT2_TGT_CDR_REC       (1 << 3)    // Targeted CDR Recording
#define CS_IDENT2_PFX_ENB           (1 << 4)    // Prefix Enable
#define CS_IDENT2_EMERG_ENB         (1 << 5)    // Emergency Number Enable
#define CS_IDENT2_XLAT_FLAG         (1 << 6)    // Translation Flag: enables the feature translation of numbers.
#define CS_IDENT2_RESERVED          (1 << 7)    // Reserved for future use.

#define CS_CALLTYPE_INCOMING        (0)
#define CS_CALLTYPE_UNANSWERED      (1)
#define CS_CALLTYPE_ABANDONED       (2)
#define CS_CALLTYPE_LOCAL           (3)
#define CS_CALLTYPE_INTRA_LATA      (4)
#define CS_CALLTYPE_INTER_LATA      (5)
#define CS_CALLTYPE_INTERNATIONAL   (6)
#define CS_CALLTYPE_OPERATOR        (7)
#define CS_CALLTYPE_ZERO_PLUS       (8)
#define CS_CALLTYPE_1800            (9)
#define CS_CALLTYPE_DA              (10)                // Directory Assistance
#define CS_CALLTYPE_DENIED          (11)
#define CS_CALLTYPE_UNASSIGNED      (12)
#define CS_CALLTYPE_UNASSIGNED2     (13)
#define CS_CALLTYPE_E_PURSE         (14)
#define CS_CALLTYPE_UNKNOWN         (15)

#define CS_CLASS_REGULAR            (0x01)
#define CS_CLASS_UNKNOWN_11         (0x11)
#define CS_CLASS_INFORMATION        (0x41)
#define CS_CLASS_OPERATOR           (0x81)
#define CS_CLASS_DISABLED           (0)

/* ADMESS (Advertising) pp. 2-5 */
#define ADMESS_ATTR_PERMANENT                   0       // Permanent
#define ADMESS_ATTR_FLASHING_SAME               1       // Flashing Same
#define ADMESS_ATTR_FLASHING_NEXT               2       // Flashing Next
#define ADMESS_ATTR_SCROLL_IN                   3       // Scroll In
#define ADMESS_ATTR_SCROLL_OUT                  4       // Scroll Out
#define ADMESS_ATTR_SCROLL_IN_AND_OUT           5       // Scroll In and Out
#define ADMESS_ATTR_NEW_PROMPT                  17      // New Prompt
#define ADMESS_ATTR_FLASHING_SAME_OFF           18      // Flashing Same Off
#define ADMESS_ATTR_FLASHING_NEXT_2ND           19      // Flashing Next 2nd
#define ADMESS_ATTR_FLASHING_NEXT_1ST_OFF       20      // Flashing Next 1st Off
#define ADMESS_ATTR_FLASHING_NEXT_2ND_OFF       21      // Flashing Next 2nd Off
#define ADMESS_ATTR_BLANK_BEFORE                0x40    //

typedef struct admess_table_entry {
    uint16_t display_time;                  /* Length of time in 10ms increments that the message displays or scrolls. */
    uint8_t  display_attr;                  /* Display attributes */
    uint8_t  spare;                         /* This seems to indicate class of number */
    uint8_t  message_text[20];              /* The Advertising message. */
} admess_table_entry_t;

#define ADVERT_PROMPTS_MAX                      20
typedef struct dlog_mt_advert_prompts {
    admess_table_entry_t entry[ADVERT_PROMPTS_MAX];
} dlog_mt_advert_prompts_t;

/* FEATRU Bit Definitions, see pp. 2-182 */
/* Data jack at offset 0x35 in FEATRU table */
#define DATAJACK_ENABLED                        (1 << 0)    /* Indicates wether terminal allows (1) or blocks (0) datajack calls. */
#define DATAJACK_MUTED_AFTER_DIAL               (1 << 1)    /* Indicates when the handset is muted for a datajack call. */
#define DATAJACK_ALLOW_FREE_LOCAL_CALLS         (1 << 2)    /* Indicates wether local datajack call are free (1) or chargeable (0). */
#define DATAJACK_ALLOW_DA_CALLS                 (1 << 3)    /* Indicates wether directory assistance calls are allowed during a datajack call. */
#define DATAJACK_FLAGS_4_7                      (0xF0)      /* Datajack flags (unused) */

/* FEATRU (Universal Feature Configuration Table) pp 2-151 */
typedef struct dlog_mt_fconfig_opts {
    uint8_t     term_type;                      /* Corresponds to the same Terminal Type which is used in the Terminal Control table. */
    uint8_t     display_present;                /* Indicates whether a Visual display is present on the terminal.  If value is “0”, Advertising Enabled should be “0”. */
    uint8_t     num_call_follows;               /* This indicates the maximum number of Follow-on calls which can be made by one user without requiring re-insertion and re-validation of card. */
    uint8_t     card_val_info;                  /* Card validation flags (see bit definitions below.) */
    uint8_t     accs_mode_info;                 /* ACCS mode flags (see bit definitions below.) */
    uint8_t     incoming_call_mode;             /* Incoming call mode (see bit definitions below.) */
    uint8_t     anti_fraud_for_incoming_call;
    uint8_t     OOS_POTS_flags;
    uint8_t     datajack_display_delay;         /* Time in seconds. Indicates the time the terminal will delay before displaying the datajack prompt. */
    uint8_t     lang_scroll_order;
    uint8_t     lang_scroll_order2;
    uint8_t     num_of_languages;
    uint8_t     rating_flags;                   /* Determines the type of set-based rating. */
    uint8_t     dialaround_timer;               /* When making a dial around call, indicates the amount of time a caller needs to hold down the # before receiving dial tone. 0 = disabled. */
    uint8_t     call_screen_list_ixl_oper_entry;
    uint8_t     call_screen_list_inter_lata_aos_entry;
    uint8_t     call_screen_list_ixl_aos_entry;
    uint8_t     datajack_grace_period;
    uint8_t     operator_collection_timer;
    uint8_t     call_screen_list_intra_lata_oper_entry;
    uint8_t     call_screen_list_inter_lata_oper_entry;
    uint8_t     advertising_flags;                  /* Advertising mode flags (see bit definitions below.) */
    uint8_t     default_language;
    uint8_t     call_setup_param_flags;             /* Call setup parameter flags (see bit definitions below.) */
    uint8_t     dtmf_duration;
    uint8_t     interdigit_pause;
    uint8_t     ppu_preauth_credit_limit;           /* SPARE K */
    uint8_t     coin_calling_features;              /* Coin calling feature flags (see bit definitions below.) */
    uint16_t    coin_call_overtime_period;          /* Time after indicated expiry of coin call before call is disconnected. This overtime period allows the user additional time to enter more coins to extend the duration of the call. The user is not billed for this additional time if they fail to enter more coins */
    uint16_t    coin_call_pots_time;                /* Time after supplementary power is lost on a coin call before it is disconnected (while in fail to POTS mode). */
    uint8_t     international_min_digits;           /* This does not include 01 or 011. Indicates the minimum number of digits required for an international call. */
    uint8_t     default_rate_req_payment_type;      /* Contains the default payment type used in the rate request before the caller choses a payment type.  Between 2 and 16 */
    uint8_t     next_call_revalidation_frequency;   /* Indicates thethe number of follow on calls that can be made before a card needs to be revalidated. */
    uint8_t     cutoff_on_disc_duration;            /* The maximum duration (* 10ms) of a C.O. line break before the terminal will disconnect the call. */
    uint16_t    cdr_upload_timer_international;     /* International Call Detail Record Upload timer. Indicates the amount of time after the CDR is created before it will automatically upload. */
    uint16_t    cdr_upload_timer_domestic;          /* Non-International Call Detail Record Upload timer. Indicates the amount of time after the CDR is created before it will automatically upload. */
    uint8_t     num_perf_stat_dialog_fails;         /* Indicates the number of dialog failures before the performance statistics message will be sent up at its regular time. Otherwise the terminal will not send up this message. */
    uint8_t     num_co_line_check_fails;
    uint8_t     num_alt_ncc_dialog_check_fails;
    uint8_t     num_failed_dialogs_until_oos;
    uint8_t     num_failed_dialogs_until_alarm;
    uint8_t     smartcard_flags;                    /* Smart Card flag bits */
    uint8_t     max_num_digits_manual_card_entry;   /* Maximum digits allowed for manually dialed card numbers. */
    uint8_t     call_screen_list_zp_aos_entry;      /* A pointer to the AOS number located in the call screening list. */
    uint8_t     carrier_reroute_flags;              /* Carrier reroute flags, see below. */
    uint8_t     min_num_digits_manual_card_entry;   /* Minimum digits allowed for manually dialed card numbers. */
    uint8_t     max_num_smartcard_inserts;          /* Maximum of total smart cards allowed to be inserted during a call. */
    uint8_t     max_num_diff_smartcard_inserts;     /* Maximum of different smart cards allowed to be inserted during a call. */
    uint8_t     call_screen_list_zm_aos_entry;      /* Is a pointer to the AOS number located in the call screening list. */
    uint8_t     datajack_flags;                     /* Datajack flags */
    uint16_t    delay_on_hook_card_alarm;
    uint16_t    delay_on_hook_card_alarm_after_call;
    uint16_t    duration_of_card_alarm;
    uint16_t    card_alarm_on_cadence;
    uint16_t    card_alarm_off_cadence;
    uint16_t    card_reader_blocked_alarm_delay;
    uint8_t     settlement_time;                    /* Check IAS Settle Time Before AS Between 0 and 255. */
    uint8_t     grace_period_domestic;              /* Check IAS Domestic Grace Period Between 0 and 255. */
    uint8_t     ias_timeout;
    uint8_t     grace_period_international;
    uint8_t     settlement_time_datajack_calls;
} dlog_mt_fconfig_opts_t;

#define FC_CARD_AUTH_ON_LOCAL_CALLS             (1 << 0)    /* Indicates whether it is necessary for the terminal to validate a card for local calls. */
#define FC_DELAYED_CARD_AUTHORIZATION           (1 << 1)    /* Indicates when to perform card authorization. */
#define FC_CARD_AUTH_ON_MCE_LOCAL_CALLS         (1 << 2)    /* Indicates whether it is necessary for the terminal to validate MCE local calls. */
#define FC_NO_NPA_ADDED_ZP_LOCAL_ACCS           (1 << 3)    /* “0” NPA added, “1” NPA not added. */
#define FC_CARD_AUTH_BIT_4                      (1 << 4)    /* “0” No Card Authorization Required, “1” Card Authorization Required. */
#define FC_CARD_AUTH_BIT_5                      (1 << 5)    /* “0” No Card Authorization Required, “1” Card Authorization Required. */
#define FC_CARD_AUTH_BIT_6                      (1 << 6)    /* “0” No Card Authorization Required, “1” Card Authorization Required. */
#define FC_IMMED_MCE_CARD_AUTH                  (1 << 7)    /* Indicates whether it is necessary for the terminal to validate immediately an MCE manually-entered card number or wait for the called number to be dialed before validating the card. */

#define FC_ACCS_AVAILABLE                       (1 << 0)    /* Indicates whether ACCS is available for card validation. */
#define FC_MCE_ROUTING                          (1 << 1)    /* “0” Routed to Millennium Manager, “1” Routed to ACCS */
#define FC_MANUAL_DIALED_CARD_NUM_ENABLED       (1 << 2)    /* Indicates whether manually dialed calling card digits are buffered by the terminal for automatic download to ACCS when using next call. */
#define FC_MANUALLY_DIALED_NCC_VALID_REQ        (1 << 3)    /* Indicates if Millennium Manager validation is required for manually dialed cardnumbers(1) or validation is not required(0). */
#define FC_AOS_ENABLED                          (1 << 4)    /* Indicates if Automated Operator Services (AOS) is enabled (1) or disabled(0). */
#define FC_ZERO_PLUS_LOCAL_CALLS_TO_NCC         (1 << 5)    /* “0” Terminal will do a 0 to 1 conversion. “1” Terminal will strip off leading 0 or 1. */
#define FC_ACCS_INFO_BIT_6                      (1 << 6)    /* Reserved */
#define FC_REMOVE_NPA_ZP_LOCAL_NCC_CALLS        (1 << 7)    /* “0” Terminal will not remove NPA “1” Terminal will remove NPA from 0+ NPA + TD calls. */
                                                            /* Remove NPA on Local Calls (Note: a value of yes indicates that the terminal is to remove the NPA from 0+NPA+7D local calls.) */
#define FC_CALL_MODE_NO_INCOMING                (0)         /* “0” Ringing Disabled, no answering of incoming data calls. */
#define FC_CALL_MODE_INCOMING_VOICE_ONLY        (1)         /* “1” Ringing Enabled, answering voice calls only. */
#define FC_CALL_MODE_RING_DISABLED_ANSWER_DATA  (2)         /* “2” Ringing Disabled, answering as data call immediately. */
#define FC_CALL_MODE_RING_ENABLED_ANSWER_DATA   (3)         /* “3” Ringing Enabled, answering data call after specified number of rings. */

#define FC_IN_SERVICE_ON_CDR_LIST_FULL          (1 << 0)    /* Flag used to determine whether the terminal will remain in service (1) or not in service (0) when the CDR list is full. */
#define FC_TERM_RATE_DISPLAY_OPTION             (1 << 1)    /* Flag used to indicate if the card terminal will display an initial rate. */
#define FC_INCOMING_CALL_FCA_PRECEDENCE         (1 << 2)    /* Flag used to tell the terminal which takes precedence, an incoming call (0) or an FCA/smart card alert(1). */
#define FC_FCA_ON_CARD                          (1 << 3)    /* Flag used to tell the terminal whether the FCA / smart card alert will sound (1) or not sound (0), for a zero-value cash card. */
#define FC_REVERT_TO_PRIMARY_NCC_NUM            (1 << 4)    /* If set, the terminal will revert to the primary Millennium Manager number at the call-in time if the terminal is currently using the secondary number. */
#define FC_BLOCK_NO_RATE_CARRIER                (1 << 5)    /* MISC BIT 5 / BLOCK NO RATE CARRIER: MTR 2.x: Block Carrier calls without internal rate. */
#define FC_RATED_CREDIT_CARD_CDR                (1 << 6)    /* MISC BIT 6 / RATED CREDIT CARD CDR: Creditcard CDRs should contain the charged amount for the calls. */
#define FC_11_DIGIT_LOCAL_CALLS                 (1 << 7)    /* MISC BIT 7 / 11 DIGIT LOCAL CALLS: MTR 2.x: Force 11-digit-dialing on local calls */

#define FC_ENABLE_NPA_SBR                       (1 << 0)    /* Enable NPA Set-based rating. */
#define FC_ENABLE_IXL_SBR                       (1 << 1)    /* Enable International Set-based rating. */
#define FC_ENABLE_DIAL_AROUND                   (1 << 2)    /* Enable dial-around timer. */
#define FC_SHOW_INIT_ADDL_RATE                  (1 << 3)    /* Display initial and additional rate. */
#define FC_ROUND_UP_CHARGE                      (1 << 4)    /* Round up the charge */
#define FC_7_DIGIT_NO_WAIT                      (1 << 5)    /* 7-Digit no-wait. */
#define FC_RATING_BIT6                          (1 << 6)
#define FC_RATING_BIT7                          (1 << 7)

#define FC_ADVERT_ENABLED                       (1 << 0)    /* Indicates whether Advertising is enabled on the terminal. */
#define FC_REP_DIALER_ADVERTISING               (1 << 1)    /* Indicates if Rep Dialer/quick access key Advertising is enabled(“1”) or disabled(“0”). */
#define FC_CALL_ESTABLISHED_ADVERTISING         (1 << 2)    /* Indicates if Call Established Advertising is enabled(“1”) or disabled(“0”). */
#define FC_ENABLE_DATE_TIME_DISPLAY             (1 << 3)    /* Indicates if date-time display is enabled. */
#define FC_TIME_FORMAT                          (1 << 4)    /* “0” 24 hour display, “1” 12 hour display. */
#define FC_ADVERTISING_FLAGS_BIT_5              (1 << 5)    /* Reserved */
#define FC_ADVERTISING_FLAGS_BIT_6              (1 << 6)    /* Reserved */
#define FC_ADVERTISING_FLAGS_BIT_7              (1 << 7)    /* Reserved */

#define FC_DISPLAY_CALLED_NUMBER                (1 << 0)    /* Indicates wether or not to display the called number. */
#define FC_ENABLE_SERVLEV_DISP_FLASHING         (1 << 1)    /* Determines if the degraded service prompts flash (1) or display solid (0). */
#define FC_CALL_SETUP_PARAMS_BIT_2              (1 << 2)    /* Reserved */
#define FC_CALL_SETUP_PARAMS_BIT_3              (1 << 3)    /* Reserved */
#define FC_CALL_SETUP_PARAMS_BIT_4              (1 << 4)    /* Reserved */
#define FC_CALL_SETUP_PARAMS_BIT_5              (1 << 5)    /* Reserved */
#define FC_CALL_SETUP_PARAMS_BIT_6              (1 << 6)    /* Reserved */
#define FC_SUPPRESS_CALLING_PROMPT              (1 << 7)    /* Indicates whether Calling prompt displays while the terminal is dialing out. */

/* Coin Calling Feature Flags */
#define FC_COIN_CALL_OVERTIME                   (1 << 0)    /* Indicates whether an overtime period on coin calls is allowed for the user to insert coins to extend call duration. */
#define FC_VOICE_FEEDBACK_ON_COIN_CALL          (1 << 1)    /* Indicates whether voice feedback is provided on coin calls */
#define FC_COIN_CALL_SECOND_WARNING             (1 << 2)    /* Indicates whether the second warning is enabled for coin calls. */
#define FC_COIN_CALL_FEATURES_BIT_3             (1 << 3)    /* Reserved */
#define FC_COIN_CALL_FEATURES_BIT_4             (1 << 4)    /* Reserved */
#define FC_COIN_CALL_FEATURES_BIT_5             (1 << 5)    /* Reserved */
#define FC_COIN_CALL_FEATURES_BIT_6             (1 << 6)    /* Reserved */
#define FC_COIN_CALL_FEATURES_BIT_7             (1 << 7)    /* Reserved */

/* Smart Card flag bits */
#define FC_SMART_CARD_FLAGS_BIT_0               (1 << 0)    /* Reserved */
#define FC_SC_VALID_INTERNATIONAL_CALLS         (1 << 1)    /* Smart Card valid for international calls. */
#define FC_SC_VALID_INTER_LATA_CALLS            (1 << 2)    /* Smart Card valid for Inter-LATA calls. */
#define FC_SC_VALID_INTRA_LATA_CALLS            (1 << 3)    /* Smart Card valid for Intra-LATA calls. */
#define FC_SC_VALID_LOCAL_CALLS                 (1 << 4)    /* Smart Card valid for Local calls. */
#define FC_POST_PAYMENT_RATE_REQUEST            (1 << 5)    /* If the payment type chosen is different than the type used by the rate request, turning on this flag will trigger a second rate request. */
#define FC_USE_TERMINAL_CARD_TABLE_DEF          (1 << 6)    /* ? */
#define FC_RATE_INFO_NOT_DISPLAYED              (1 << 7)    /* ? */

/* Carrier Re-Route flags */
#define FC_BLOCK_REROUTE_COIN_CALL              (1 << 0)    /* Blocks (0) or redirects (1) invalid carrier coin calls. */
#define FC_BLOCK_REROUTE_CREDIT_CARD_CALL       (1 << 1)    /* Blocks (0) or redirects (1) invalid carrier credit card calls. */
#define FC_BLOCK_REROUTE_SMART_CARD_CALL        (1 << 2)    /* Blocks (0) or redirects (1) invalid carrier smart card calls. */
#define FC_BLOCK_REROUTE_CALL_CARD_CALL         (1 << 3)    /* Blocks (0) or redirects (1) invalid carrier calling card calls. */
#define FC_CARRIER_BLOCK_REROUTE_BIT_4          (1 << 4)    /* Reserved */
#define FC_CARRIER_BLOCK_REROUTE_BIT_5          (1 << 5)    /* Reserved */
#define FC_CARRIER_BLOCK_REROUTE_BIT_6          (1 << 6)    /* Reserved */
#define FC_CARRIER_BLOCK_REROUTE_BIT_7          (1 << 7)    /* Reserved */

/* Datajack flags */
#define FC_DATAJACK_ENABLED                     (1 << 0)    /* Indicates wether terminal allows (1) or blocks (0) datajack calls. */
#define FC_DATAJACK_MUTING                      (1 << 1)    /* Indicates when the handset is muted for a datajack call. */
#define FC_DATAJACK_ALLOW_FREE_LOCAL_CALL       (1 << 2)    /* Indicates wether local datajack call are free (1) or chargeable (0). */
#define FC_DATAJACK_ALLOW_DA_CALLS              (1 << 3)    /* Indicates wether directory assistance calls are allowed during a datajack call. */
#define FC_DJ_FLAGS_BIT_4                       (1 << 4)    /* Reserved */
#define FC_DJ_FLAGS_BIT_5                       (1 << 5)    /* Reserved */
#define FC_DJ_FLAGS_BIT_6                       (1 << 6)    /* Reserved */
#define FC_DJ_FLAGS_BIT_7                       (1 << 7)    /* Reserved */

/* Installation and Servicing Table (INSTSV) pp. 2.236 */
/* also: https://wiki.millennium.management/dlog:dlog_mt_install_parms */
typedef struct dlog_mt_install_params {
    uint8_t access_code[4];                     /* This value will be compared to a password entered by the maintenance person, to verify access to the terminal. */
    uint8_t key_card_number[5];                 /* This is the telephone number to be dialed by the terminal when it is being tested by the installer or maintenance person. It is usually the number of a maintenance phone for the Telco. */
    uint8_t flags;                              /* See bit definitions below. */
    uint8_t tx_packet_delay;                    /* in 10ms increments, This is the amount of time that the terminal should pause between packet transmissions to the Millennium Manager. This is used to assist the Millennium Manager in recognizing when the end of a packet has been reached and a new packet is being received. */
    uint8_t rx_packet_gap;                      /* in 10ms increments, This is the amount of time delay that the Millennium Manager will insert prior to the frame character at the start of a packet. It is used by the terminal when receiving the packet, to distinguish a frame character at the start of a packet from characters which may be part of the data inside the packet. */
    uint8_t retries_until_oos;                  /* This is the number of failed attempts to contact the Millennium Manager that the terminal will make before switching to its out-of-service condition. */
    uint8_t coin_service_flags;                 /* Coin servicing flags: Cashbox Query Menu is Accessible via craft interface */
    uint16_t coinbox_lock_timeout;              /* Time in seconds from when the cash box lock is opened until an alarm is sent to the Millennium Manager. This is a timeout on the time required to collect the cash box. */
    uint8_t predial_string[4];                  /* Predial string for the primary NCC. */
    uint8_t predial_string_alt[4];              /* TPredial string for the secondary NCC. */
    uint8_t spare[12];                          /* Some used for MTR2.x. */
} dlog_mt_install_params_t;

#define ACCESS_CODE_LEN                         (7)

#define INSTSV_PREDIAL_STRING_ENABLE            (1 << 0)    /* Predial string enable. */
#define INSTSV_PREDIAL_STRING_ENABLE_1P         (1 << 1)    /* Predial string enable for 1+ calls. */
#define INSTSV_PREDIAL_STRING_ENABLE_IXL        (1 << 2)    /* Predial string enable for International calls. */
#define INSTSV_PREDIAL_STRING_ENABLE_ALL        (1 << 3)    /* Predial string enabled for all all except free calls. */

#define INSTSV_CASHBOX_QUERY_MENU_ENABLE        (1 << 0)    /* Cashbox Query Menu is Accessible via craft interface. */

/* See TSTATUS, 2-637 */
#define TSTATUS_HANDSET_DISCONT_IND             (1 << 0)
#define TSTATUS_TELEPHONY_STATUS_IND            (1 << 1)
#define TSTATUS_EPM_SAM_NOT_RESPONDING          (1 << 2)
#define TSTATUS_EPM_SAM_LOCKED_OUT              (1 << 3)
#define TSTATUS_EPM_SAM_EXPIRED                 (1 << 4)
#define TSTATUS_EPM_SAM_REACHING_TRANS_LIMIT    (1 << 5)
#define TSTATUS_UNABLE_REACH_PRIM_COL_SYS       (1 << 6)
#define TSTATUS_TELEPHONY_STATUS_BIT_7          (1 << 7)
#define TSTATUS_POWER_FAIL_IND                  (1 << 8)
#define TSTATUS_DISPLAY_RESPONSE_IND            (1 << 9)
#define TSTATUS_VOICE_SYNTHESIS_RESPONSE_IND    (1 << 10)
#define TSTATUS_UNABLE_REACH_SECOND_COL_SYS     (1 << 11)
#define TSTATUS_CARD_READER_BLOCKED_ALARM       (1 << 12)
#define TSTATUS_MANDATORY_TABLE_ALARM           (1 << 13)
#define TSTATUS_DATAJACK_PORT_BLOCKED           (1 << 14)
#define TSTATUS_CTRL_HW_STATUS_BIT_7            (1 << 15)
#define TSTATUS_CDR_CHECKSUM_ERR_IND            (1 << 16)
#define TSTATUS_STATISTICS_CHECKSUM_ERR_IND     (1 << 17)
#define TSTATUS_TERMINAL_TBL_CHECKSUM_ERR_IND   (1 << 18)
#define TSTATUS_OTHER_DATA_CHECKSUM_ERR_IND     (1 << 19)
#define TSTATUS_CDR_LIST_FULL_ERR_IND           (1 << 20)
#define TSTATUS_BAD_EEPROM_ERR_IND              (1 << 21)
#define TSTATUS_MEMORY_LOST_ERROR_IND           (1 << 22)
#define TSTATUS_MEMORY_BAD_ERR_IND              (1 << 23)
#define TSTATUS_ACCESS_COVER_IND                (1 << 24)
#define TSTATUS_KEY_MATRIX_MALFUNC_IND          (1 << 25)
#define TSTATUS_SET_REMOVAL_IND                 (1 << 26)
#define TSTATUS_THRESHOLD_MET_EXCEEDED_IND      (1 << 27)
#define TSTATUS_CASH_BOX_COVER_OPEN_IND         (1 << 28)
#define TSTATUS_CASH_BOX_REMOVED_IND            (1 << 29)
#define TSTATUS_COIN_BOX_FULL_IND               (1 << 30)
#define TSTATUS_COIN_JAM_COIN_CHUTE_IND         (1 << 31)
#define TSTATUS_ESCROW_JAM_IND                  (1 << 32)
#define TSTATUS_VAL_HARDWARE_FAIL_IND           (1 << 33)
#define TSTATUS_CO_LINE_CHECK_FAIL_IND          (1 << 34)
#define TSTATUS_DIALOG_FAILURE_IND              (1 << 35)
#define TSTATUS_CASH_BOX_ELECTRONIC_LOCK_IND    (1 << 36)
#define TSTATUS_DIALOG_FAILURE_WITH_COL_SYS     (1 << 37)
#define TSTATUS_CODE_SERVE_CONNECTION_FAILURE   (1 << 38)
#define TSTATUS_CODE_SERVER_ABORTED             (1 << 39)

/* DLOG_MT_TERM_STATUS */
typedef struct dlog_mt_term_status {
    uint8_t serialnum[5];
    uint8_t status[5];
} dlog_mt_term_status_t;

/* Smart Card Definitions */
#define SC_REBATE_INTRALATA         0   /* Amount deducted from rate for smart card intralatacalls. */
#define SC_REBATE_IXL               1   /* Amount deducted from rate for smart card international calls. */
#define SC_REBATE_LOCAL             2   /* Amount deducted from rate for smart card local calls. */
#define SC_REBATE_INTERLATA         3   /* Amount deducted from rate for smart card interlata calls. */
#define SC_REBATE_DA                4   /* Amount deducted from rate for smart card Directory assistance calls. */
#define SC_REBATE_1800              5   /* Amount deducted from rate for smart card 1-800 calls. */
#define SC_REBATE_LOCAL_LMS_ADL     6   /* Amounted deducted from the rate for local (LMS) additional rate rebate. */
#define SC_REBATE_INTRALATA_ADL     7   /* Amount deducted from the rate for the intra-lata additional rate rebate. */
#define SC_REBATE_INTERLATA_ADL     8   /* Amount deducted from the rate for the inter-lata additional rate rebate. */
#define SC_REBATE_IXL_ADL           9   /* Amount deducted from the rate for international additional rate rebate. */
#define SC_REBATE_DA_ADL            10  /* Amount deducted from the rate for the directory assistance additional rate rebate. */
#define SC_REBATE_1800_ADL          11  /* Amount deducted from the rate for the 1-800 additional rate rebate. */
#define SC_SURCHARGE_DATAJACK       12  /* Initial surcharge for a datajack call. */
#define SC_SURCHARGE_DATAJACK_ADL   13  /* Additional surcharge for a datajack call. */

#define SC_MULT_MAX_UNIT_MAX        15
#define SC_REBATE_MAX               14

#define SC_MAX_UNIT_MASK            (0x3FFF)
#define SC_MULT_MASK                (~SC_MAX_UNIT_MASK)
#define SC_MULT_SHIFT               (14)
#define SC_DES_KEY_MAX              20
#define SC_DES_KEY_LEN              8

typedef struct des_key {
    uint8_t x[SC_DES_KEY_LEN];
} des_key_t;

/* SMCARD (Smart Card Parameters Table) pp 2-351 */
typedef struct dlog_mt_scard_parm_table {
    des_key_t   des_key[SC_DES_KEY_MAX];                /* 20 8-byte DES keys */
    uint16_t    mult_max_unit[SC_MULT_MAX_UNIT_MAX];    /* 15 Mult / Max Unit entries */
    uint16_t    rebates[SC_REBATE_MAX];                 /* Rebate table (14 entries) */
    uint8_t     spare[6];                               /* Spare */
} dlog_mt_scard_parm_table_t;

#define TABLE_PATH_MAX_LEN   283

typedef struct mm_context {
    int fd;
    FILE *logstream;
    FILE *bytestream;
    FILE *cdr_stream;
    char phone_rev;
    char terminal_id[11];   /* The terminal's phone number */
    char ncc_number[2][21];
    char default_table_dir[256];
    char term_table_dir[256];
    uint8_t rx_seq;
    uint8_t tx_seq;
    uint8_t first_chunk;
    uint8_t table_num;
    int table_offset;
    uint8_t table[10 * 1024];
    uint8_t cdr_ack_buffer[PKT_TABLE_DATA_LEN_MAX];
    uint8_t cdr_ack_buffer_len;
    int table_len;
    uint8_t trans_data_in_progress;
    uint8_t curr_table;
    uint8_t use_modem;
    uint8_t debuglevel;
    uint8_t connected;
    uint8_t minimal_table_set;
    dlog_mt_install_params_t instsv;
    cashbox_status_univ_t cashbox_status;
} mm_context_t;



/* MM Table Operations */
int receive_mm_table(mm_context_t *context, mm_table_t *table);
int mm_download_tables(mm_context_t *context);
int send_mm_table(mm_context_t *context, uint8_t* payload, int len);
int wait_for_table_ack(mm_context_t *context, uint8_t table_id);
int load_mm_table(mm_context_t *context, uint8_t table_id, uint8_t **buffer, int *len);
int rewrite_instserv_parameters(char *access_code, dlog_mt_install_params_t *pinstsv_table, char *filename);
void generate_term_access_parameters(mm_context_t* context, uint8_t** buffer, int* len);
void generate_call_in_parameters(mm_context_t* context, uint8_t** buffer, int* len);
void generate_call_stat_parameters(mm_context_t *context, uint8_t **buffer, int *len);
void generate_comm_stat_parameters(mm_context_t *context, uint8_t **buffer, int *len);
void generate_user_if_parameters(mm_context_t *context, uint8_t **buffer, int *len);
void generate_dlog_mt_end_data(mm_context_t *context, uint8_t **buffer, int *len);
int update_terminal_cash_box_staus_table(mm_context_t *context, cashbox_status_univ_t *cashbox_status);

/* MM Protocol */
extern int receive_mm_packet(mm_context_t *context, mm_packet_t *pkt);
extern int send_mm_packet(mm_context_t *context, uint8_t *payload, int len, uint8_t flags);
extern int send_mm_ack(mm_context_t *context, uint8_t flags);
extern int wait_for_mm_ack(mm_context_t *context);
extern int print_mm_packet(int direction, mm_packet_t *pkt);

/* modem functions: */
extern int init_modem(int fd);
extern int wait_for_connect(int fd);
extern int hangup_modem(int fd);

/* mm_util */
extern unsigned crc16(unsigned crc, uint8_t *buf, size_t len);
extern void dump_hex(uint8_t *data, size_t len);
extern char *phone_num_to_string(char *string_buf, size_t string_len, uint8_t* num_buf, size_t num_buf_len);
extern uint8_t string_to_bcd_a(char* number_string, uint8_t* buffer, uint8_t buff_len);
extern char *callscrn_num_to_string(char *string_buf, size_t string_buf_len, uint8_t* num_buf, size_t num_buf_len);
extern char *call_type_to_string(uint8_t call_type, char *string_buf, size_t string_buf_len);
extern char *timestamp_to_string(uint8_t *timestamp, char *string_buf, size_t string_buf_len);
extern void print_bits(uint8_t bits, char *str_array[]);
extern char *table_to_string(uint8_t table);

#pragma pack(pop)
