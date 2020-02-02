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
#define DLOG_MT_ADVERT_PROMPTS      0x1d    // 29: Advertising prompts
#define DLOG_MT_INSTALL_PARAMS      0x1f    // 31: Installation/Servicing Parameters
#define DLOG_MT_TIME_SYNC_REQ       0x24    // 36: Time Synchronization Request
#define DLOG_MT_PERF_STATS_MSG      0x25    // 37: Performance Statistics Record Message
#define DLOG_MT_CASH_BOX_STATUS     0x26    // 38: Cash Box Status Message – Universal
#define DLOG_MT_ATN_REQ_CDR_UPL     0x2a    // 42: Attention Request Call Records Upload
#define DLOG_MT_ATN_REQ_TAB_UPD     0x2c    // 44: Attention Request Table Update
#define DLOG_MT_CASH_BOX_COLLECTION 0x33    // 51: Cash Box Collection Message – Universal
#define DLOG_MT_CALL_DETAILS        0x35    // 53: Call Detail Record Message – Post MSR 1.5
#define DLOG_MT_SUMMARY_CALL_STATS  0x38    // 56: Summary Call Statistics Record – Post MSR 1.6
#define DLOG_MT_CARRIER_CALL_STATS  0x39    // 57: Carrier Call Statistics Message
#define DLOG_MT_SW_VERSION          0x3c    // 60: Terminal Software Version Message
#define DLOG_MT_RATE_REQUEST        0x3f    // 63: Rate Request Message – Universal
#define DLOG_MT_RATE_RESPONSE       0x40    // 64: Rate Response Message – Universal
#define DLOG_MT_CARRIER_STATS_EXP   0x47    // 71: Expanded Carrier Call Statistics Message

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

typedef struct mm_context {
    int fd;
    FILE *logstream;
    FILE *bytestream;
    char phone_rev;
    char phone_number[11];
    char access_code[8];
    char ncc_number[2][21];
    uint8_t rx_seq;
    uint8_t tx_seq;
    uint8_t first_chunk;
    uint8_t table_num;
    int table_offset;
    uint8_t table[10 * 1024];
    int table_len;
    uint8_t curr_table;
    uint8_t use_modem;
    uint8_t debuglevel;
    uint8_t connected;
} mm_context_t;

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

typedef struct dlog_mt_call_details {
    uint8_t rate_type;
    uint8_t called_num[10];
    uint8_t carrier_code;
    uint8_t other_num[10];
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
    uint8_t pad[85];
} dlog_mt_perf_stats_record_t;

/* DLOG_MT_SUMMARY_CALL_STATS */
typedef struct dlog_mt_summary_call_stats {
    uint8_t stats[80];
} dlog_mt_summary_call_stats_t;

/* DLOG_MT_CARRIER_CALL_STATS 106 bytes */
typedef struct dlog_mt_carrier_call_stats {
    uint8_t timestamp[6];
    uint8_t timestamp2[6];
    uint8_t pad[94];
} dlog_mt_carrier_call_stats_t;

/* DLOG_MT_CARRIER_STATS_EXP */
typedef struct dlog_mt_carrier_stats_exp {
    uint8_t timestamp[6];
    uint8_t timestamp2[6];
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
    mm_local                // 8 Millennium Manager rated local
} rate_type_t;

typedef struct rate_table_entry {
    uint8_t type;
    uint16_t initial_period;
    uint16_t initial_charge;
    uint16_t additional_period;
    uint16_t additional_charge;
} rate_table_entry_t;

/* DLOG_MT_RATE_REQUEST */
typedef struct dlog_mt_rate_request {
    uint16_t seq;
    uint8_t phone_number[10];
    uint8_t timestamp[6];
    uint8_t pad[6];
} dlog_mt_rate_request_t;

/* DLOG_MT_RATE_RESPONSE  (24 bytes) */
/* DLOG_MT_RATE_RESPONSE  (25 bytes) */
typedef struct dlog_mt_rate_response {
    rate_table_entry_t rate;
    uint8_t pad2[16];
} dlog_mt_rate_response_t;

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

/* CALLSCR (Call Screening List) pp. 2-33 */
typedef struct call_screen_list_entry {
    uint8_t  pad[4];                        /* Need to figure out what these are for. */
    uint8_t  phone_number[9];               /* 0-terminated phone number, one digit per nibble. */
    uint8_t  class;                         /* This seems to indicate class of number */
    uint8_t  spare[3];                      /* These might be spares, they are all 0's. */
} call_screen_list_entry_t;

#define CALLSCRN_TABLE_MAX                      180
typedef struct dlog_mt_call_screen_list {
    call_screen_list_entry_t entry[CALLSCRN_TABLE_MAX];
} dlog_mt_call_screen_list_t;

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
} dlog_mt_cadvert_prompts_t;

/* FEATRU Bit Definitions, see pp. 2-182 */
/* Data jack at offset 0x35 in FEATRU table */
#define DATAJACK_ENABLED                        (1 << 0)    /* Indicates wether terminal allows (1) or blocks (0) datajack calls. */
#define DATAJACK_MUTED_AFTER_DIAL               (1 << 1)    /* Indicates when the handset is muted for a datajack call. */
#define DATAJACK_ALLOW_FREE_LOCAL_CALLS         (1 << 2)    /* Indicates wether local datajack call are free (1) or chargeable (0). */
#define DATAJACK_ALLOW_DA_CALLS                 (1 << 3)    /* Indicates wether directory assistance calls are allowed during a datajack call. */
#define DATAJACK_FLAGS_4_7                      (0xF0)      /* Datajack flags (unused) */

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

/* MM Table Operations */
int receive_mm_table(mm_context_t *context, mm_table_t *table);
int mm_download_tables(mm_context_t *context);
int send_mm_table(mm_context_t *context, uint8_t* payload, int len, int end_of_data);
int wait_for_table_ack(mm_context_t *context, uint8_t table_id);
int load_mm_table(uint8_t table_id, uint8_t **buffer, int *len);
int rewrite_instserv_parameters(mm_context_t *context, uint8_t *table_buffer, int table_len);
int rewrite_term_access_parameters(mm_context_t *context, uint8_t *table_buffer, int table_len);

/* MM Protocol */
extern int receive_mm_packet(mm_context_t *context, mm_packet_t *pkt);
extern int send_mm_packet(mm_context_t *context, uint8_t *payload, int len, uint8_t flags);
extern int send_mm_ack(mm_context_t *context, uint8_t flags);
extern int wait_for_mm_ack(mm_context_t *context);
extern int print_mm_packet(int direction, mm_packet_t *pkt);

/* modem functions: */
extern int open_port(char *modem_dev);
extern int init_port(int fd, int baudrate);
extern int init_modem(int fd);
extern int wait_for_connect(int fd);
extern int hangup_modem(int fd);

/* mm_util */
extern unsigned crc16(unsigned crc, uint8_t *buf, size_t len);
extern void dump_hex(uint8_t *data, int len);
extern char *phone_num_to_string(char *string_buf, int string_len, uint8_t* num_buf, int num_buf_len);
extern char *callscrn_num_to_string(char *string_buf, int string_buf_len, uint8_t* num_buf, int num_buf_len);
extern char *call_type_to_string(uint8_t call_type, char *string_buf, int string_buf_len);

#pragma pack(pop)
