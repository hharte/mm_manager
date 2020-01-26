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

typedef struct dlog_mt_call_details {
    uint8_t rate_type;
    uint8_t called_num[10];
    uint8_t carrier_code;
    uint8_t other_num[10];
    uint32_t call_cost[2];
    uint16_t seq;
    uint8_t start_timestamp[6];
    uint8_t call_duration[3];
    uint8_t pad3[12];
} dlog_mt_call_details_t;

/* DLOG_MT_CASH_BOX_COLLECTION */
typedef struct dlog_mt_cash_box_collection {
    uint8_t pad[14];
    uint8_t timestamp[6];
    uint8_t cash[50];
} dlog_mt_cash_box_collection_t;

/* TABLE_ID_CASHBOX_STATUS_UNIV */
typedef struct cashbox_status_univ {
    uint8_t timestamp[6];
    uint8_t status[50];
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

/* DLOG_MT_RATE_REQUEST */
typedef struct dlog_mt_rate_request {
    uint16_t seq;
    uint8_t phone_number[10];
    uint8_t timestamp[6];
    uint8_t pad[6];
} dlog_mt_rate_request_t;

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
extern int send_mm_packet(mm_context_t *context, uint8_t *payload, int len);
extern int send_mm_ack(mm_context_t *context);
extern int wait_for_mm_ack(mm_context_t *context);
extern int print_mm_packet(int direction, mm_packet_t *pkt);

/* modem functions: */
extern int open_port(char *modem_dev);
extern int init_port(int fd);
extern int init_modem(int fd);
extern int wait_for_connect(int fd);
extern int hangup_modem(int fd);

/* mm_util */
extern unsigned crc16(unsigned crc, uint8_t *buf, size_t len);
extern void dump_hex(uint8_t *data, int len);
extern char *phone_num_to_string(char *string_buf, int string_len, uint8_t* num_buf, int num_buf_len);

#pragma pack(pop)
