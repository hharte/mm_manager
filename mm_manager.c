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

#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <errno.h>   /* Error number definitions */
#include <time.h>    /* time_t, struct tm, time, gmtime */
#include <libgen.h>
#include <sys/stat.h>

#include "mm_manager.h"

#define JAN12020 1577865600

/* Terminal Table Lists for Rev 1.3 and Rev 1.0 Control PCP */
uint8_t table_list_rev1_3[] = {
    DLOG_MT_INTL_SBR_TABLE,
    DLOG_MT_NPA_SBR_TABLE,
    DLOG_MT_NPA_NXX_TABLE_16,
    DLOG_MT_NPA_NXX_TABLE_15,
    DLOG_MT_NPA_NXX_TABLE_14,
    DLOG_MT_NPA_NXX_TABLE_13,
    DLOG_MT_NPA_NXX_TABLE_12,
    DLOG_MT_NPA_NXX_TABLE_11,
    DLOG_MT_NPA_NXX_TABLE_10,
    DLOG_MT_NPA_NXX_TABLE_9,
    DLOG_MT_NPA_NXX_TABLE_8,
    DLOG_MT_NPA_NXX_TABLE_7,
    DLOG_MT_NPA_NXX_TABLE_6,
    DLOG_MT_NPA_NXX_TABLE_5,
    DLOG_MT_NPA_NXX_TABLE_4,
    DLOG_MT_NPA_NXX_TABLE_3,
    DLOG_MT_NPA_NXX_TABLE_2,
    DLOG_MT_NPA_NXX_TABLE_1,    /* Required */
    DLOG_MT_CARRIER_TABLE,
    DLOG_MT_CARD_TABLE,         /* Required */
    DLOG_MT_SCARD_PARM_TABLE,
    DLOG_MT_CALL_SCREEN_LIST,
    DLOG_MT_VIS_PROPTS_L2,
    DLOG_MT_VIS_PROPTS_L1,
    DLOG_MT_RATE_TABLE,         /* Required */
    DLOG_MT_SPARE_TABLE,
    DLOG_MT_NUM_PLAN_TABLE,
    DLOG_MT_LIMSERV_DATA,
    DLOG_MT_REP_DIAL_LIST,
    DLOG_MT_COIN_VAL_TABLE,     /* Required */
    DLOG_MT_CALL_IN_PARMS,
    DLOG_MT_CALL_STAT_PARMS,
    DLOG_MT_MODEM_PARMS,
    DLOG_MT_COMM_STAT_PARMS,
    DLOG_MT_INSTALL_PARAMS,     /* Required */
    DLOG_MT_USER_IF_PARMS,
    DLOG_MT_ADVERT_PROMPTS,
    DLOG_MT_FCONFIG_OPTS,       /* Required */
    DLOG_MT_NCC_TERM_PARAMS,    /* Required */
    DLOG_MT_END_DATA,
    0                           /* End of table list */
};

uint8_t table_list_rev1_0[] = {
    DLOG_MT_INTL_SBR_TABLE,
    DLOG_MT_NPA_SBR_TABLE,
    DLOG_MT_NPA_NXX_TABLE_16,
    DLOG_MT_NPA_NXX_TABLE_15,
    DLOG_MT_NPA_NXX_TABLE_14,
    DLOG_MT_NPA_NXX_TABLE_13,
    DLOG_MT_NPA_NXX_TABLE_12,
    DLOG_MT_NPA_NXX_TABLE_11,
    DLOG_MT_NPA_NXX_TABLE_10,
    DLOG_MT_NPA_NXX_TABLE_9,
    DLOG_MT_NPA_NXX_TABLE_8,
    DLOG_MT_NPA_NXX_TABLE_7,
    DLOG_MT_NPA_NXX_TABLE_6,
    DLOG_MT_NPA_NXX_TABLE_5,
    DLOG_MT_NPA_NXX_TABLE_4,
    DLOG_MT_NPA_NXX_TABLE_3,
    DLOG_MT_NPA_NXX_TABLE_2,
    DLOG_MT_NPA_NXX_TABLE_1,    /* Required */
    DLOG_MT_CARRIER_TABLE,
    DLOG_MT_CARD_TABLE,         /* Required */
    DLOG_MT_SCARD_PARM_TABLE,
    DLOG_MT_CALL_SCREEN_LIST,
    DLOG_MT_RATE_TABLE,         /* Required */
    DLOG_MT_NUM_PLAN_TABLE,
    DLOG_MT_LIMSERV_DATA,
    DLOG_MT_REP_DIAL_LIST,
    DLOG_MT_COIN_VAL_TABLE,     /* Required */
    DLOG_MT_CALL_IN_PARMS,
    DLOG_MT_CALL_STAT_PARMS,
    DLOG_MT_MODEM_PARMS,
    DLOG_MT_COMM_STAT_PARMS,
    DLOG_MT_INSTALL_PARAMS,     /* Required */
    DLOG_MT_USER_IF_PARMS,
    DLOG_MT_ADVERT_PROMPTS,
    DLOG_MT_FCONFIG_OPTS,       /* Required */
    DLOG_MT_NCC_TERM_PARAMS,    /* Required */
    DLOG_MT_END_DATA,
    0                           /* End of table list */
};

uint8_t table_list_minimal[] = {
    DLOG_MT_NPA_NXX_TABLE_1,    /* Required */
    DLOG_MT_CARRIER_TABLE,
    DLOG_MT_CARD_TABLE,         /* Required */
    DLOG_MT_SCARD_PARM_TABLE,
    DLOG_MT_CALL_SCREEN_LIST,
    DLOG_MT_RATE_TABLE,         /* Required */
    DLOG_MT_NUM_PLAN_TABLE,
    DLOG_MT_COIN_VAL_TABLE,     /* Required */
    DLOG_MT_INSTALL_PARAMS,     /* Required */
    DLOG_MT_FCONFIG_OPTS,       /* Required */
    DLOG_MT_NCC_TERM_PARAMS,    /* Required */
    DLOG_MT_END_DATA,
    0                           /* End of table list */
};


/* Millennium Alarm Strings */
const char *alarm_type_str[] = {
/* 0 */     "Telephony Board Not Responding",   // TSTATUS_HANDSET_DISCONT_IND
/* 1 */     "TELEPHONY_STATUS_IND",
/* 2 */     "EPM/SAM Not Responding",
/* 3 */     "EPM/SAM Locked Out",
/* 4 */     "EPM/SAM Expired",
/* 5 */     "EPM/SAM has reached the transaction limit",
/* 6 */     "Unable to Reach Primary Collection System",
/* 7 */     "Reserved TELEPHONY_STATUS_BIT_7",
/* 8 */     "Power Fail",
/* 9 */     "Display Not Responding",
/* 10 */    "Voice Synthesis Not Responding",
/* 11 */    "Unable to Reach Secondary Collection System",
/* 12 */    "Card Reader Blocked",
/* 13 */    "Mandatory Table Alarm",
/* 14 */    "Datajack Port Blocked",
/* 15 */    "Reserved CTRL_HW_STATUS_BIT_7",
/* 16 */    "CDR Checksum Error",
/* 17 */    "Statistics Checksum Error",
/* 18 */    "Table Checksum Error",
/* 19 */    "Data Checksum Error",
/* 20 */    "CDR List Full",
/* 21 */    "Bad EEPROM",
/* 22 */    "Control Microprocessor RAM Contents Lost",
/* 23 */    "Control Microprocessor RAM Defective",
/* 24 */    "Station Access Cover Opened",
/* 25 */    "Stuck Button",
/* 26 */    "Set Removal",    /* Not all terminals have this switch sensor */
/* 27 */    "Cash Box Threshold Met",
/* 28 */    "Coin Box Cover Opened",
/* 29 */    "Cash Box Removed",
/* 30 */    "Cash Box Full",
/* 31 */    "Validator Jam",
/* 32 */    "Escrow Jam",
/* 33 */    "Coin Hardware Jam",
/* 34 */    "Central Office Line Check Failure",
/* 35 */    "Dialog Failure",
/* 36 */    "Cash Box Electronic Lock Failure", // What is this?
/* 37 */    "Dialog Failure with Collection System",
/* 38 */    "Code Server Connection Failure",
/* 39 */    "Code Server Aborted",
/* ... */
/* 99  */   "Un-Alarm",
/* >39 */   "Unknown Alarm!"
};


int main(int argc, char *argv[])
{
    FILE *instream;
    mm_context_t mm_context;
    mm_table_t mm_table;
    int status;
    char *modem_dev = NULL;
    char *table_dir = NULL;
    int index;
    int c;
    uint8_t *instsv_table_buffer;
    int table_len;
    int baudrate = 19200;
    char access_code_str[8];

    time_t rawtime;
    struct tm *ptm;

    opterr = 0;
    mm_context.logstream = NULL;
    mm_context.cdr_stream = NULL;
    mm_context.use_modem = 0;
    mm_context.tx_seq = 0;
    mm_context.debuglevel = 0;
    mm_context.connected = 0;
    mm_context.terminal_id[0] = '\0';

    printf("mm_manager v0.5 [%s] - (c) 2020, Howard M. Harte\n\n", VERSION);

    index = 0;
    mm_context.ncc_number[0][0] = '\0';
    mm_context.ncc_number[1][0] = '\0';

    strcpy(mm_context.default_table_dir, "tables/default");
    strcpy(mm_context.term_table_dir, "tables");
    mm_context.minimal_table_set = 0;

    while ((c = getopt (argc, argv, "rvmb:c:d:l:f:ha:n:st:")) != -1) {
        switch (c)
        {
            case 'h':
                printf("usage: %s [-vhm] [-f <filename>] [-l <logfile>] [-a <access_code>] [-n <ncc_number>] [-d <default_table_dir] [-t <term_table_dir>]\n", basename(argv[0]));
                printf("\t-v verbose (multiple v's increase verbosity.)\n" \
                       "\t-d default_table_dir - default table directory.\n" \
                       "\t-f <filename> modem device or file\n" \
                       "\t-h this help.\n"
                       "\t-c <cdrfile.csv> - Write Call Detail Records to a CSV file.\n" \
                       "\t-l <logfile> - log bytes transmitted to and received from the terminal.  Useful for debugging.\n" \
                       "\t-m use serial modem (specify device with -f)\n" \
                       "\t-b <baudrate> - Modem baud rate, in bps.  Defaults to 19200.\n" \
                       "\t-n <Primary NCC Number> [-n <Secondary NCC Number>] - specify primary and optionally secondary NCC number.\n" \
                       "\t-s small - Download only minimum required tables to terminal.\n" \
                       "\t-t term_table_dir - terminal-specific table directory.\n");
                       return(0);
                       break;
            case 'a':
                if (strlen(optarg) != 7) {
                    fprintf(stderr, "Option -a takes a 7-digit access code.\n");
                    return(-1);
                }

                if (load_mm_table(&mm_context, DLOG_MT_INSTALL_PARAMS, &instsv_table_buffer, &table_len)) {
                    fprintf (stderr, "Error reading install parameters from %s/mm_table_1f.bin.\n", mm_context.default_table_dir);
                    return -1;
                }
                memcpy(&mm_context.instsv, instsv_table_buffer+1, sizeof(dlog_mt_install_params_t));
                free(instsv_table_buffer);

                if (rewrite_instserv_parameters(optarg, &mm_context.instsv, "tables/mm_table_1f.bin")) {
                    printf("Error updating INSTSV parameters\n");
                    return (-1);
                }
                break;
            case 'c':
                if ((mm_context.cdr_stream = fopen(optarg, "a+")) < 0) {
                    (void)fprintf(stderr,
                        "mm_manager: %s: %s\n", optarg, strerror(errno));
                    exit(1);
                }
                fseek(mm_context.cdr_stream, 0, SEEK_END);
                if (ftell(mm_context.cdr_stream) == 0) {
                    printf("Creating CDR file '%s'\n", optarg);
                    fprintf(mm_context.cdr_stream, "TERMINAL_ID,SEQ,TIMESTAMP,DURATION,TYPE,DIALED_NUM,CARD,REQUESTED,COLLECTED,CARRIER,RATE\n");
                    fflush(mm_context.cdr_stream);
                } else {
                    printf("Appending to existing CDR file '%s'\n", optarg);
                }
                break;
            case 'v':
                mm_context.debuglevel++;
                break;
            case 'f':
                modem_dev = optarg;
                break;
            case 'l':
                if ((mm_context.logstream = fopen(optarg, "w")) < 0) {
                    (void)fprintf(stderr,
                        "mm_manager: %s: %s\n", optarg, strerror(errno));
                    exit(1);
                }
                break;
            case 'm':
                mm_context.use_modem = 1;
                break;
            case 'b':
                baudrate = atoi(optarg);
                break;
            case 'n':
                if(index > 1) {
                    fprintf(stderr, "-n may only be specified twice.\n");
                    return(-1);
                }
                if ((strlen(optarg) < 1) || (strlen(optarg) > 15)) {
                    fprintf(stderr, "Option -n takes a 1- to 15-digit NCC number.\n");
                    return(-1);
                } else {
                    strncpy(mm_context.ncc_number[index], optarg, sizeof(mm_context.ncc_number[0]));
                    index++;
                }
                break;
            case 's':
                printf("NOTE: Using minimum required table list for download.\n");
                mm_context.minimal_table_set = 1;
                break;
            case 'd':
                strcpy(mm_context.default_table_dir, optarg);
                break;
            case 't':
                strcpy(mm_context.term_table_dir, optarg);
                break;
            case '?':
                if (optopt == 'f' || optopt == 'l' || optopt == 'a' || optopt == 'n' || optopt == 'b')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);

                return 1;
                break;
            default:
                return(-1);
        }
    }

    for (index = optind; index < argc; index++) {
        printf ("Non-option argument %s\n", argv[index]);
        return 0;
    }

    printf("Default Table directory: %s\n", mm_context.default_table_dir);
    printf("Terminal-specific Table directory: %s/<terminal_id>\n", mm_context.term_table_dir);

    if (load_mm_table(&mm_context, DLOG_MT_INSTALL_PARAMS, &instsv_table_buffer, &table_len)) {
        fprintf (stderr, "Error reading install parameters from %s/mm_table_1f.bin.\n", mm_context.default_table_dir);
        return -1;
    }
    memcpy(&mm_context.instsv, instsv_table_buffer+1, sizeof(dlog_mt_install_params_t));
    free(instsv_table_buffer);

    printf("Using access code: %s\n", phone_num_to_string(access_code_str, sizeof(access_code_str), mm_context.instsv.access_code, sizeof(mm_context.instsv.access_code)));
    printf("Manager Inter-packet Tx gap: %dms.\n", mm_context.instsv.rx_packet_gap * 10);

    if(strlen(mm_context.ncc_number[0]) >= 1) {
        printf("Using Primary NCC number: %s\n", mm_context.ncc_number[0]);

        if(strlen(mm_context.ncc_number[1]) == 0) {
            strncpy(mm_context.ncc_number[1], mm_context.ncc_number[0], sizeof(mm_context.ncc_number[1]));
        }

        printf("Using Secondary NCC number: %s\n", mm_context.ncc_number[1]);
    } else if (mm_context.use_modem == 1) {
        fprintf(stderr, "Error: -n <NCC Number> must be specified.\n");
        return(-1);
    }

    if (mm_context.use_modem == 0) {
        if(!(mm_context.bytestream = fopen(modem_dev, "r"))) {
            printf("Error opening input stream: %s\n", modem_dev);
            exit(-1);
        }
        mm_context.connected = 1;
    } else {
        if (baudrate < 1200) {
            printf("Error: baud rate must be 1200 bps or faster.\n");
            return(-1);
        }
        printf("Baud Rate: %d\n", baudrate);

        mm_context.fd = open_port(modem_dev);
        init_port(mm_context.fd, baudrate);
        status = init_modem(mm_context.fd);
        if (status == 0) {
            printf("Modem initialized.\n");
        } else {
            printf("Error initializing modem.\n");
            return (-1);
        }
    }

    while(1) {
        if(mm_context.use_modem == 1) {
            printf("Waiting for call from terminal...\n");
            if(wait_for_connect(mm_context.fd) == 0) {

                mm_context.tx_seq = 0;
                time ( &rawtime );
                ptm = localtime ( &rawtime );

                printf("%04d-%02d-%02d %2d:%02d:%02d: Connected!\n\n",
                    ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

                mm_context.connected = 1;
            } else {
                printf("Timed out waiting for connect, retrying...\n");
                continue;
            }
        }

        while((receive_mm_table(&mm_context, &mm_table) == 0) && (mm_context.connected == 1)) {
        }
        if (mm_context.use_modem == 1) {
            time(&rawtime);
        } else {
            rawtime = JAN12020;
        }

        ptm = localtime ( &rawtime );

        printf("\n\n%04d-%02d-%02d %2d:%02d:%02d: Disconnected.\n\n",
            ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    }

    return 0;
}

int receive_mm_table(mm_context_t *context, mm_table_t *table)
{
    mm_packet_t *pkt = &table->pkt;
    int i;
    uint8_t ack_payload[245] = { 0 };
    uint8_t *pack_payload = ack_payload;
    char serial_number[11];
    char terminal_version[12];
    char timestamp_str[20];
    char timestamp2_str[20];
    uint8_t *ppayload;
    uint8_t cashbox_pending = 0;
    uint8_t table_download_pending = 0;
    uint8_t dont_send_reply = 0;
    uint8_t end_of_data = 0;
    uint8_t status;

    if ((status = receive_mm_packet(context, pkt)) != 0) {
        if ((status & PKT_ERROR_DISCONNECT) == 0) {
            send_mm_ack(context, FLAG_RETRY);   /* Retry unless the terminal disconnected. */
        }
        return 0;
    }

    context->rx_seq = pkt->hdr.flags & FLAG_SEQUENCE;

    ppayload = pkt->payload;

    if(pkt->payload_len < PKT_TABLE_ID_OFFSET) {
        table->table_id = 0;
        print_mm_packet(RX, pkt);
        printf("Error: Received an ACK without expecting it!\n");
        return 0;
    }

    phone_num_to_string(context->terminal_id, sizeof(context->terminal_id), ppayload, PKT_TABLE_ID_OFFSET);
    ppayload += PKT_TABLE_ID_OFFSET;

    if (context->debuglevel > 0) {
        print_mm_packet(RX, pkt);
    }

    /* Acknowledge the received packet */
    send_mm_ack(context, 0);

    while (ppayload < pkt->payload + pkt->payload_len) {

        table->table_id = *ppayload++;
        if (context->debuglevel > 1) printf("\n\tTerminal ID %s: Processing Table ID %d (0x%02x) %s\n", context->terminal_id, table->table_id, table->table_id, table_to_string(table->table_id));

        switch(table->table_id) {
            case DLOG_MT_TIME_SYNC_REQ: {
                time_t rawtime;
                struct tm *ptm;
                uint8_t *timestamp;

                *pack_payload++ = DLOG_MT_TIME_SYNC;
                timestamp = pack_payload;

                if (context->use_modem == 1) {
                    /* When using the modem, fill the current time.  If not using the modem, use
                    * a static time, so that results can be automatically checked.
                    */
                    time(&rawtime);
                } else {
                    rawtime = JAN12020;
                }

                ptm = localtime(&rawtime);

                *pack_payload++ = (ptm->tm_year & 0xff);     /* Fill current years since 1900 */
                *pack_payload++ = (ptm->tm_mon+1 & 0xff);    /* Fill current month (1-12) */
                *pack_payload++ = (ptm->tm_mday & 0xff);     /* Fill current day (0-31) */
                *pack_payload++ = (ptm->tm_hour & 0xff);     /* Fill current hour (0-23) */
                *pack_payload++ = (ptm->tm_min & 0xff);      /* Fill current minute (0-59) */
                *pack_payload++ = (ptm->tm_sec & 0xff);      /* Fill current second (0-59) */
                *pack_payload++ = (ptm->tm_wday + 1);        /* Day of week, 1=Sunday ... 7=Saturday */

                printf("\t\tCurrent day/time: %04d-%02d-%02d / %2d:%02d:%02d\n", timestamp[0]+1900,
                                                                            timestamp[1],
                                                                            timestamp[2],
                                                                            timestamp[3],
                                                                            timestamp[4],
                                                                            timestamp[5]);

                end_of_data = 1;
                break;
            }
            case DLOG_MT_ATN_REQ_TAB_UPD: {
                uint8_t terminal_upd_reason;

                terminal_upd_reason = *ppayload++;
                printf("\t\tTerminal requests table update. Reason: 0x%02x [%s%s%s%s%s]\n\n", terminal_upd_reason,
                        terminal_upd_reason & TTBLREQ_CRAFT_FORCE_DL ? "Force Download, " : "",
                        terminal_upd_reason & TTBLREQ_CRAFT_INSTALL  ? "Install, " : "",
                        terminal_upd_reason & TTBLREQ_LOST_MEMORY    ? "Lost Memory, " : "",
                        terminal_upd_reason & TTBLREQ_PWR_LOST_ON_DL ? "Power Lost on Download, " : "",
                        terminal_upd_reason & TTBLREQ_CASHBOX_STATUS ? "Cashbox Status Request" : "");

                cashbox_pending = 1;
                table_download_pending = 1;
                break;
            }
            case DLOG_MT_ALARM: {
                dlog_mt_alarm_t *alarm = (dlog_mt_alarm_t *)ppayload;
                uint8_t alarm_index;

                ppayload += sizeof(dlog_mt_alarm_t);

                *pack_payload++ = DLOG_MT_ALARM_ACK;
                *pack_payload++ = alarm->alarm_id;

                /* Our Alarm Table only has 41 entries, if alarm is 99 (Un-Alarm) set it to 40,
                    * if our alarm is > 39 (last valid alarm, except 99) then use alarm 41 to display
                    * "Unknown Alarm."
                    */
                if (alarm->alarm_id == 99) {
                    alarm_index = 40;
                } else if (alarm->alarm_id > 39) {
                    alarm_index = 41;
                } else {
                    alarm_index = alarm->alarm_id;
                }

                printf("\t\tAlarm: %s: Type: %d (0x%02x) - %s\n",
                    timestamp_to_string(alarm->timestamp, timestamp_str, sizeof(timestamp_str)),
                    alarm->alarm_id, alarm->alarm_id,
                    alarm_type_str[alarm_index]);
                break;
            }
            case DLOG_MT_MAINT_REQ: {
                dlog_mt_maint_req_t *maint = (dlog_mt_maint_req_t *)ppayload;;
                ppayload += sizeof(dlog_mt_maint_req_t);

                printf("\t\tMaintenance Type: %d (0x%03x) Access PIN: %02x%02x%01x\n",
                    maint->type, maint->type,
                    maint->access_pin[0], maint->access_pin[1], (maint->access_pin[2] & 0xF0) >> 4);

                *pack_payload++ = DLOG_MT_MAINT_ACK;
                *pack_payload++ = maint->type & 0xFF;
                *pack_payload++ = (maint->type >> 8) & 0xFF;
                break;
            }
            case DLOG_MT_CALL_DETAILS: {
                dlog_mt_call_details_t *cdr = (dlog_mt_call_details_t *)ppayload;
                char phone_number_string[21];
                char card_number_string[21];
                char call_type_str[38];

                ppayload += sizeof(dlog_mt_call_details_t);

                printf("\t\tCDR: %s, Duration: %02d:%02d:%02d %s, DN: %s, Card#: %s, Collected: $%3.2f, Requested: $%3.2f, carrier code=%d, rate_type=%d, Seq: %04d\n",
                    timestamp_to_string(cdr->start_timestamp, timestamp_str, sizeof(timestamp_str)),
                    cdr->call_duration[0],
                    cdr->call_duration[1],
                    cdr->call_duration[2],
                    call_type_to_string(cdr->call_type, call_type_str, sizeof(call_type_str)),
                    phone_num_to_string(phone_number_string, sizeof(phone_number_string), cdr->called_num, sizeof(cdr->called_num)),
                    phone_num_to_string(card_number_string, sizeof(card_number_string), cdr->card_num, sizeof(cdr->card_num)),
                    (float)cdr->call_cost[0] / 100,
                    (float)cdr->call_cost[1] / 100,
                    cdr->carrier_code,
                    cdr->rate_type,
                    cdr->seq);

                if (context->cdr_stream != NULL) {
                    fprintf(context->cdr_stream, "%s,%d,%s,%d,%s,%s,%s,$%3.2f,$%3.2f,%d,%d\n",
                        context->terminal_id,
                        cdr->seq,
                        timestamp_to_string(cdr->start_timestamp, timestamp_str, sizeof(timestamp_str)),
                        cdr->call_duration[0] * 3600 +
                        cdr->call_duration[1] * 60 +
                        cdr->call_duration[2],
                        call_type_to_string(cdr->call_type, call_type_str, sizeof(call_type_str)),
                        phone_num_to_string(phone_number_string, sizeof(phone_number_string), cdr->called_num, sizeof(cdr->called_num)),
                        phone_num_to_string(card_number_string, sizeof(card_number_string), cdr->card_num, sizeof(cdr->card_num)),
                        (float)cdr->call_cost[1] / 100,
                        (float)cdr->call_cost[0] / 100,
                        cdr->carrier_code,
                        cdr->rate_type);
                    fflush(context->cdr_stream);
                }

                if (context->debuglevel > 2) {
                    printf("\t\t\tDLOG_MT_CALL_DETAILS Pad:");
                    dump_hex(cdr->pad, sizeof(cdr->pad));
                }

                *pack_payload++ = DLOG_MT_CDR_DETAILS_ACK;
                *pack_payload++ = cdr->seq & 0xFF;
                *pack_payload++ = (cdr->seq >> 8) & 0xFF;
                break;
            }
            case DLOG_MT_ATN_REQ_CDR_UPL: {
                /* Not sure what the cdr_req_type is, just swallow it. */
                uint8_t cdr_req_type = *ppayload++;
                printf("\t\tSeq: %d: DLOG_MT_ATN_REQ_CDR_UPL, cdr_req_type=%02x (0x%02x)\n", context->tx_seq, cdr_req_type, cdr_req_type);

                *pack_payload++ = DLOG_MT_TRANS_DATA;
                break;
            }
            case DLOG_MT_CASH_BOX_COLLECTION: {
                dlog_mt_cash_box_collection_t *cash_box_collection = (dlog_mt_cash_box_collection_t *)ppayload;
                printf("\t\tSeq: %d: DLOG_MT_CASH_BOX_COLLECTION.\n", context->tx_seq);

                ppayload += sizeof(dlog_mt_cash_box_collection_t);

                printf("\t\t\t%s: Total: $%3.2f (%3d%% full): CA N:%d D:%d Q:%d $:%d - US N:%d D:%d Q:%d $:%d\n",
                        timestamp_to_string(cash_box_collection->timestamp, timestamp_str, sizeof(timestamp_str)),
                        (float)cash_box_collection->currency_value / 100,
                        cash_box_collection->percent_full,
                        cash_box_collection->coin_count[COIN_COUNT_CA_NICKELS],
                        cash_box_collection->coin_count[COIN_COUNT_CA_DIMES],
                        cash_box_collection->coin_count[COIN_COUNT_CA_QUARTERS],
                        cash_box_collection->coin_count[COIN_COUNT_CA_DOLLARS],
                        cash_box_collection->coin_count[COIN_COUNT_US_NICKELS],
                        cash_box_collection->coin_count[COIN_COUNT_US_DIMES],
                        cash_box_collection->coin_count[COIN_COUNT_US_QUARTERS],
                        cash_box_collection->coin_count[COIN_COUNT_US_DOLLARS]);
                break;
            }
            case DLOG_MT_TERM_STATUS: {
                dlog_mt_term_status_t *dlog_mt_term_status = (dlog_mt_term_status_t *)ppayload;
                uint8_t serial_number[11];
                uint64_t dlog_mt_term_status_word;
                int i;

                ppayload += sizeof(dlog_mt_term_status_t);

                for (i=0; i < 5; i++) {
                    serial_number[i*2] = ((dlog_mt_term_status->serialnum[i] & 0xf0) >> 4) + '0';
                    serial_number[i*2+1] = (dlog_mt_term_status->serialnum[i] & 0x0f) + '0';
                }

                serial_number[10] = '\0';

                dlog_mt_term_status_word  = dlog_mt_term_status->status[0];
                dlog_mt_term_status_word |= dlog_mt_term_status->status[1] << 8;
                dlog_mt_term_status_word |= dlog_mt_term_status->status[2] << 16;
                dlog_mt_term_status_word |= dlog_mt_term_status->status[3] << 24;
                dlog_mt_term_status_word |= (uint64_t)(dlog_mt_term_status->status[4]) << 32;

                printf("\t\tTerminal serial number %s, Terminal Status Word: 0x%010llx\n",
                    serial_number, dlog_mt_term_status_word);

                /* Iterate over all the terminal status bits and display a message for any flags set. */
                for (i = 0; dlog_mt_term_status_word != 0; i++) {
                    if (dlog_mt_term_status_word & 1) {
                        printf("\t\t\tTerminal Status: %s\n", alarm_type_str[i]);
                    }
                    dlog_mt_term_status_word >>= 1;
                }
                break;
            }
            case DLOG_MT_SW_VERSION: {
                dlog_mt_sw_version_t *dlog_mt_sw_version = (dlog_mt_sw_version_t *)ppayload;

                char control_rom_edition[sizeof(dlog_mt_sw_version->control_rom_edition) + 1] = { 0 };
                char control_version[sizeof(dlog_mt_sw_version->control_version) + 1] = { 0 };
                char telephony_rom_edition[sizeof(dlog_mt_sw_version->telephony_rom_edition) + 1] = { 0 };
                char telephony_version[sizeof(dlog_mt_sw_version->telephony_version) + 1] = { 0 };
                memcpy(control_rom_edition, dlog_mt_sw_version->control_rom_edition, sizeof(dlog_mt_sw_version->control_rom_edition));
                memcpy(control_version, dlog_mt_sw_version->control_version, sizeof(dlog_mt_sw_version->control_version));
                memcpy(telephony_rom_edition, dlog_mt_sw_version->telephony_rom_edition, sizeof(dlog_mt_sw_version->telephony_rom_edition));
                memcpy(telephony_version, dlog_mt_sw_version->telephony_version, sizeof(dlog_mt_sw_version->telephony_version));

                ppayload += sizeof(dlog_mt_sw_version_t);

                if (strcmp(control_version, "V1.0") == 0) {
                    context->phone_rev = 10;
                } else if (strcmp(control_version, "V1.1") == 0) {
                    context->phone_rev = 13;
                } else if (strcmp(control_version, "V1.3") == 0) {
                    context->phone_rev = 13;
                } else {
                    printf("Error: Unknown control version %s, defaulting to tables for V1.0.\n", control_version);
                    context->phone_rev = 10;
                }

                printf("\t\t\t             Terminal Type: %02d (0x%02x)\n", dlog_mt_sw_version->term_type, dlog_mt_sw_version->term_type);
                printf("\t\t\t       Control ROM Edition: %s\n", control_rom_edition);
                printf("\t\t\t           Control Version: %s\n", control_version);
                printf("\t\t\t     Telephony ROM Edition: %s\n", telephony_rom_edition);
                printf("\t\t\t         Telephony Version: %s\n", telephony_version);
                printf("\t\t\tValidator Hardware Version: %c%c\n", dlog_mt_sw_version->validator_hw_ver[0], dlog_mt_sw_version->validator_hw_ver[1]);
                printf("\t\t\tValidator Software Version: %c%c\n", dlog_mt_sw_version->validator_sw_ver[0], dlog_mt_sw_version->validator_sw_ver[1]);

                break;
            }
            case DLOG_MT_CASH_BOX_STATUS: {
                cashbox_status_univ_t *cashbox_status = &context->cashbox_status;

                /* Save cashbox status in our context */
                memcpy(cashbox_status, ppayload, sizeof(cashbox_status_univ_t));

                ppayload += sizeof(cashbox_status_univ_t);

                update_terminal_cash_box_staus_table(context, &context->cashbox_status);
                printf("\t\tCashbox status: %s: Total: $%3.2f (%3d%% full): CA N:%d D:%d Q:%d $:%d - US N:%d D:%d Q:%d $:%d\n",
                        timestamp_to_string(cashbox_status->timestamp, timestamp_str, sizeof(timestamp_str)),
                        (float)cashbox_status->currency_value / 100,
                        cashbox_status->percent_full,
                        cashbox_status->coin_count[COIN_COUNT_CA_NICKELS],
                        cashbox_status->coin_count[COIN_COUNT_CA_DIMES],
                        cashbox_status->coin_count[COIN_COUNT_CA_QUARTERS],
                        cashbox_status->coin_count[COIN_COUNT_CA_DOLLARS],
                        cashbox_status->coin_count[COIN_COUNT_US_NICKELS],
                        cashbox_status->coin_count[COIN_COUNT_US_DIMES],
                        cashbox_status->coin_count[COIN_COUNT_US_QUARTERS],
                        cashbox_status->coin_count[COIN_COUNT_US_DOLLARS]);
                break;
            }
            case DLOG_MT_PERF_STATS_MSG: {
                dlog_mt_perf_stats_record_t *perf_stats = (dlog_mt_perf_stats_record_t *)ppayload;
                ppayload += sizeof(dlog_mt_perf_stats_record_t);

                printf("\t\tPerformance Statistics Record: From: %s, to: %s:\n",
                        timestamp_to_string(perf_stats->timestamp, timestamp_str, sizeof(timestamp_str)),
                        timestamp_to_string(perf_stats->timestamp2, timestamp2_str, sizeof(timestamp2_str)));
                break;
            }
            case DLOG_MT_CALL_IN: {
                printf("\tSeq: %d: DLOG_MT_CALL_IN.\n", context->tx_seq);
                *pack_payload++ = DLOG_MT_TRANS_DATA;
                break;
            }
            case DLOG_MT_CALL_BACK: {
                printf("\tSeq: %d: DLOG_MT_CALL_BACK.\n", context->tx_seq);
                *pack_payload++ = DLOG_MT_TRANS_DATA;
                break;
            }
            case DLOG_MT_CARRIER_CALL_STATS:
            {
                dlog_mt_carrier_call_stats_t *carr_stats = (dlog_mt_carrier_call_stats_t *)ppayload;
                ppayload += sizeof(dlog_mt_carrier_call_stats_t);
                printf("\tSeq: %d: DLOG_MT_CARRIER_CALL_STATS.\n", context->tx_seq);
                printf("\t\tCarrier Call Statistics Record: From: %s, to: %s:\n",
                        timestamp_to_string(carr_stats->timestamp, timestamp_str, sizeof(timestamp_str)),
                        timestamp_to_string(carr_stats->timestamp2, timestamp2_str, sizeof(timestamp2_str)));

                printf("\t\t\tCarrier 0x%02x\n", carr_stats->carrier_stats[0].carrier_ref);
                printf("\t\t\tCarrier 0x%02x\n", carr_stats->carrier_stats[1].carrier_ref);
                printf("\t\t\tCarrier 0x%02x\n", carr_stats->carrier_stats[2].carrier_ref);
                break;
            }
            case DLOG_MT_CARRIER_STATS_EXP: {
                dlog_mt_carrier_stats_exp_t *carr_stats = (dlog_mt_carrier_stats_exp_t *)ppayload;
                ppayload += sizeof(dlog_mt_carrier_stats_exp_t);
                printf("\tSeq: %d: DLOG_MT_CARRIER_STATS_EXP.\n", context->tx_seq);
                printf("\t\tExpanded Carrier Statistics: From: %s, to: %s:\n",
                        timestamp_to_string(carr_stats->timestamp, timestamp_str, sizeof(timestamp_str)),
                        timestamp_to_string(carr_stats->timestamp2, timestamp2_str, sizeof(timestamp2_str)));

                dump_hex((uint8_t *)carr_stats->unknown, sizeof(carr_stats->unknown));

                break;
            }
            case DLOG_MT_SUMMARY_CALL_STATS: {
                dlog_mt_summary_call_stats_t *dlog_mt_summary_call_stats = (dlog_mt_summary_call_stats_t *)ppayload;
                ppayload += sizeof(dlog_mt_summary_call_stats_t);

                printf("Summary call stats:\n");
                dump_hex((uint8_t *)dlog_mt_summary_call_stats, sizeof(dlog_mt_summary_call_stats));

                break;
            }
            case DLOG_MT_RATE_REQUEST: {
                char phone_number[21] = { 0 };
                dlog_mt_rate_response_t rate_response = { 0 };
                dlog_mt_rate_request_t *dlog_mt_rate_request = (dlog_mt_rate_request_t *)ppayload;
                ppayload += sizeof(dlog_mt_rate_request_t);

                phone_num_to_string(phone_number, sizeof(phone_number), dlog_mt_rate_request->phone_number, sizeof(dlog_mt_rate_request->phone_number));
                printf("\t\tRate request: %s: Phone number: %s, seq=%d, %d,%d,%d,%d,%d,%d.\n",
                        timestamp_to_string(dlog_mt_rate_request->timestamp, timestamp_str, sizeof(timestamp_str)),
                        phone_number,
                        dlog_mt_rate_request->seq,
                        dlog_mt_rate_request->pad[0],
                        dlog_mt_rate_request->pad[1],
                        dlog_mt_rate_request->pad[2],
                        dlog_mt_rate_request->pad[3],
                        dlog_mt_rate_request->pad[4],
                        dlog_mt_rate_request->pad[5]);

                rate_response.rate.type = (uint8_t)mm_local;
                rate_response.rate.initial_period = 60;
                rate_response.rate.initial_charge = 125;
                rate_response.rate.additional_period = 120;
                rate_response.rate.additional_charge = 35;
                *pack_payload++ = DLOG_MT_RATE_RESPONSE;
                memcpy(pack_payload, &rate_response, sizeof(rate_response));
                pack_payload += sizeof(rate_response);
                break;
            }
            case DLOG_MT_FUNF_CARD_AUTH: {
                char phone_number_string[21] = { 0 };
                char card_number_string[25] = { 0 };
                char call_type_str[38] = { 0 };

                dlog_mt_auth_resp_code_t auth_response = { 0 };
                dlog_mt_funf_card_auth_t *auth_request = (dlog_mt_funf_card_auth_t *)ppayload;
                ppayload += sizeof(dlog_mt_funf_card_auth_t);

                phone_num_to_string(phone_number_string, sizeof(phone_number_string), auth_request->phone_number, sizeof(auth_request->phone_number));
                phone_num_to_string(card_number_string, sizeof(card_number_string), auth_request->card_number, sizeof(auth_request->card_number));
                call_type_to_string(auth_request->call_type, call_type_str, sizeof(call_type_str));

                printf("\t\tCard Auth request: Phone number: %s, seq=%d, card#: %s, exp: %02x/%02x, init: %02x/%02x, ctrlflag: 0x%02x carrier: %d, Call_type: %s, card_ref_num:0x%02x, unk:0x%04x, unk2:0x%04x\n",
                        phone_number_string,
                        auth_request->seq,
                        card_number_string,
                        auth_request->exp_mm,
                        auth_request->exp_yy,
                        auth_request->init_mm,
                        auth_request->init_yy,
                        auth_request->control_flag,
                        auth_request->carrier_ref,
                        call_type_str,
                        auth_request->card_ref_num,
                        auth_request->unknown,
                        auth_request->unknown2);

                auth_response.resp_code = 0;
                *pack_payload++ = DLOG_MT_AUTH_RESP_CODE;
                memcpy(pack_payload, &auth_response, sizeof(auth_response));
                pack_payload += sizeof(auth_response);
                break;
            }
            case DLOG_MT_END_DATA:
                printf("\tSeq: %d: DLOG_MT_END_DATA.\n", context->tx_seq);
                *pack_payload++ = DLOG_MT_END_DATA;
                break;
            case DLOG_MT_TAB_UPD_ACK:
                printf("\tSeq: %d: DLOG_MT_TAB_UPD_ACK for table 0x%02x.\n", context->tx_seq, *ppayload);
                ppayload++;
                *pack_payload++ = DLOG_MT_TRANS_DATA;
                break;
            default:
                printf("* * * Unhandled table %d (0x%02x)", table->table_id, table->table_id);
                send_mm_ack(context, 0);
                break;
        }
    }

    /* Send cash box status if requested by terminal */
    if (cashbox_pending == 1) {
        int table_len;
        uint8_t *table_buffer;

        printf("\tSeq %d: Send DLOG_MT_CASH_BOX_STATUS table as requested by terminal.\n\t", context->tx_seq);
        if (load_mm_table(context, DLOG_MT_CASH_BOX_STATUS, &table_buffer, &table_len) == 0) {
            memcpy(pack_payload, table_buffer, table_len);
            free(table_buffer);
            pack_payload += table_len;
        } else { /* No cashbox staus saved for this terminal, use default */
            cashbox_status_univ_t empty_cashbox_status = {
                .timestamp = { 99, 1, 1, 0, 0, 0 }, /* January 1, 1999 00:00:00 */
                {0}
            };

            *pack_payload++ = DLOG_MT_CASH_BOX_STATUS;
            memcpy(pack_payload, &empty_cashbox_status, sizeof(cashbox_status_univ_t));
            pack_payload += sizeof(cashbox_status_univ_t);
        }
    }

    if (dont_send_reply == 0) {
        if (pack_payload - ack_payload == 0) {
            *pack_payload++ = DLOG_MT_END_DATA;
        }

        send_mm_table(context, ack_payload, (pack_payload - ack_payload));

        if (end_of_data == 1) {
            uint8_t end_of_data_msg = DLOG_MT_END_DATA;
            send_mm_table(context, &end_of_data_msg, sizeof(end_of_data_msg));
        }
    }
    if (table_download_pending == 1) {
        mm_download_tables(context);
    }

    return 0;
}


int mm_download_tables(mm_context_t *context)
{
    uint8_t table_data = DLOG_MT_TABLE_UPD;
    int table_index;
    int status;
    int table_len;
    uint8_t *table_buffer;
    uint8_t *table_list = table_list_rev1_3;

    /* Rev 1 PCP does not accept table 0x048, 0x55, 0x56 */
    if (context->phone_rev == 10) table_list = table_list_rev1_0;

    /* If -s was specified, download only the minimal config */
    if (context->minimal_table_set == 1) table_list = table_list_minimal;

    send_mm_table(context, &table_data, 1);

    for (table_index = 0; table_list[table_index] > 0; table_index++) {

        switch(table_list[table_index]) {
            case DLOG_MT_CALL_IN_PARMS:
                status = generate_call_in_parameters(context, &table_buffer, &table_len);
                break;
            case DLOG_MT_NCC_TERM_PARAMS:
                status = generate_term_access_parameters(context, &table_buffer, &table_len);
                break;
            default:
                printf("\t");
                status = load_mm_table(context, table_list[table_index], &table_buffer, &table_len);

                /* If table can't be loaded, continue to the next. */
                if (status != 0) continue;
                break;
        }
        send_mm_table(context, table_buffer, table_len);

        /* For all tables except END_OF_DATA, expect a table ACK. */
        if (table_list[table_index] != DLOG_MT_END_DATA) {
            wait_for_table_ack(context, table_buffer[0]);
        }
        free(table_buffer);
    }

    return 0;
}


int send_mm_table(mm_context_t *context, uint8_t *payload, int len)
{
    int i;
    mm_packet_t pkt;
    int bytes_remaining;
    int chunk_len;
    uint8_t *p = payload;
    uint8_t table_id;

    table_id = payload[0];
    bytes_remaining = len;
    printf("\tSending Table ID %d (0x%02x) %s...\n", table_id, table_id, table_to_string(table_id));
    while (bytes_remaining > 0) {
        if (bytes_remaining > 245) {
            chunk_len = 245;
        } else {
            chunk_len = bytes_remaining;
        }

        send_mm_packet(context, p, chunk_len, 0);

        if (wait_for_mm_ack(context) != 0) return -1;
        p += chunk_len;
        bytes_remaining -= chunk_len;
        printf("\tTable %d (0x%02x) progress: (%3ld%%) - %ld / %d \n", table_id, table_id, ((p - payload) * 100) / len, p - payload, len);
    }

    return 0;
}


int wait_for_table_ack(mm_context_t *context, uint8_t table_id)
{
    mm_packet_t packet;
    mm_packet_t *pkt = &packet;
    int i, status;

    if (context->debuglevel > 1) printf("Waiting for ACK for table %d (0x%02x)\n", table_id, table_id);

    if ((status = receive_mm_packet(context, pkt)) == PKT_SUCCESS) {
        context->rx_seq = pkt->hdr.flags & FLAG_SEQUENCE;

        if (pkt->payload_len >= PKT_TABLE_ID_OFFSET) {
            for (i = 0; i < PKT_TABLE_ID_OFFSET; i++) {
                context->terminal_id[i*2] = ((pkt->payload[i] & 0xf0) >> 4) + '0';
                context->terminal_id[i*2+1] = (pkt->payload[i] & 0x0f) + '0';
            }
            context->terminal_id[10] = '\0';
            if (context->debuglevel > 1) printf("Received packet from phone# %s\n", context->terminal_id);

            if((pkt->payload[PKT_TABLE_ID_OFFSET] == DLOG_MT_TAB_UPD_ACK) &&
               (pkt->payload[6] == table_id)) {
                if (context->debuglevel > 0) printf("Seq: %d: Received ACK for table %d (0x%02x)\n", context->rx_seq, table_id, table_id);
                send_mm_ack(context, 0);
            } else {
                printf("%s: ERROR: Received ACK for wrong table, expected %d (0x%02x), received %d (0x%02x)\n",
                    __FUNCTION__, table_id, table_id, pkt->payload[6], pkt->payload[6]);
                return (-1);
            }

        }
    } else {
        printf("%s: ERROR: Did not receive ACK for table ID %d (0x%02x), status=%02x\n",
            __FUNCTION__, table_id, table_id, status);
    }
    return status;
}


int load_mm_table(mm_context_t *context, uint8_t table_id, uint8_t **buffer, int *len)
{
    FILE *stream;
    char fname[TABLE_PATH_MAX_LEN];
    uint8_t c;
    long size;
    uint8_t *bufp;

    if (context->terminal_id[0] != '\0') {
        snprintf(fname, sizeof(fname), "%s/%s/mm_table_%02x.bin", context->term_table_dir, context->terminal_id, table_id);
    } else {
        snprintf(fname, sizeof(fname), "%s/mm_table_%02x.bin", context->default_table_dir, table_id);
    }

    if(!(stream = fopen(fname, "rb"))) {
        snprintf(fname, sizeof(fname), "%s/mm_table_%02x.bin", context->default_table_dir, table_id);
        if(!(stream = fopen(fname, "rb"))) {
            printf("Could not load table %d from %s.\n", table_id, fname);
            return -1;
        }
    }

    fseek(stream, 0, SEEK_END);
    size = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    size ++;    // Make room for table ID.
    *buffer = calloc(size, sizeof(uint8_t));
    fflush(stdout);
    if(*buffer == 0) {
        printf("Erorr: failed to allocate %ld bytes for table %d\n", size, table_id);
        return -1;
    }

    bufp = *buffer;
    *bufp++ = table_id;

    *len = 1;

    while (1) {

        c = fgetc(stream);
        if(feof(stream)) {
            break;
        }
        *bufp++ = c;
        *len = *len + 1;
    }

    printf("Loaded table ID %d (0x%02x) from %s (%d bytes).\n", table_id, table_id, fname, *len - 1);
    fclose(stream);

    return 0;
}

int rewrite_instserv_parameters(char *access_code, dlog_mt_install_params_t *pinstsv_table, char *filename)
{
    FILE *ostream = NULL;

    int i;
    if (strlen(access_code) != 7) {
        printf("Error: Access Code must be 7-digits\n");
    }

    // Rewrite table with our Access Code
    for (i = 0; i < (strlen(access_code)); i++) {
        if (i % 2 == 0) {
            pinstsv_table->access_code[i >> 1]  = (access_code[i] - '0') << 4;
        } else {
            pinstsv_table->access_code[i >> 1] |= (access_code[i] - '0');
        }
    }
    pinstsv_table->access_code[3] |= 0x0e;   /* Terminate the Access Code with 0xe */

    if(!(ostream = fopen(filename, "wb"))) {
        printf("Error writing %s\n", filename);
        return -1;
    }

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("Rewriting %s with access code: %s\n", filename, access_code);

        fwrite(pinstsv_table, sizeof(dlog_mt_install_params_t), 1, ostream);
        fclose(ostream);
    }

    return 0;
}

int generate_term_access_parameters(mm_context_t *context, uint8_t **buffer, int *len)
{
    int i;
    dlog_mt_ncc_term_params_t *pncc_term_params;
    uint8_t *pbuffer;

    *len = sizeof(dlog_mt_ncc_term_params_t) + 1;
    pbuffer = calloc(1, *len);
    pbuffer[0] = DLOG_MT_NCC_TERM_PARAMS;

    pncc_term_params = (dlog_mt_ncc_term_params_t *)&pbuffer[1];

    printf("\nGenerating Terminal Access Parameters table:\n" \
           "\t  Terminal ID: %s\n", context->terminal_id);

    // Rewrite table with our Terminal ID (phone number)
    for (i = 0; i < PKT_TABLE_ID_OFFSET; i++) {
        pncc_term_params->terminal_id[i]  = (context->terminal_id[i * 2    ] - '0') << 4;
        pncc_term_params->terminal_id[i] |= (context->terminal_id[i * 2 + 1] - '0');
    }

    // Rewrite table with Primary NCC phone number
    printf("\t  Primary NCC: %s\n", context->ncc_number[0]);
    string_to_bcd_a(context->ncc_number[0], pncc_term_params->pri_ncc_number, sizeof(pncc_term_params->pri_ncc_number));

    // Rewrite table with Secondary NCC phone number, if provided.
    if (strlen(context->ncc_number[1]) > 0) {
        printf("\tSecondary NCC: %s\n", context->ncc_number[1]);
        string_to_bcd_a(context->ncc_number[1], pncc_term_params->sec_ncc_number, sizeof(pncc_term_params->sec_ncc_number));
    }

    *buffer = pbuffer;

    return 0;
}

int generate_call_in_parameters(mm_context_t* context, uint8_t** buffer, int* len)
{
    int i;
    dlog_mt_call_in_params_t* pcall_in_params;
    uint8_t* pbuffer;
    time_t rawtime;
    struct tm* ptm;
    uint8_t call_in_hour;

    *len = sizeof(dlog_mt_call_in_params_t) + 1;
    pbuffer = calloc(1, *len);
    pbuffer[0] = DLOG_MT_CALL_IN_PARMS;

    pcall_in_params = (dlog_mt_call_in_params_t*)&pbuffer[1];

    printf("\nGenerating Call-In table:\n");

    if (context->use_modem == 1) {
        /* When using the modem, fill the current time.  If not using the modem, use
        * a static time, so that results can be automatically checked.
        */
        time(&rawtime);
    }
    else {
        rawtime = JAN12020;
    }

    ptm = localtime(&rawtime);

    /* Interestingly, the terminal will call in starting at the call-in time, and continue
     * calling in at intervals specified, up until midnight.  After that, the terminal will
     * not call in until the call-in time the following day.  Since we want to call in twice
     * a day, set the call-in hour to a time in the AM, so the subsequent call 12 hours later
     * will be in the PM of the same day.
     */
    call_in_hour = (ptm->tm_hour & 0xff);
    if (call_in_hour >= 12) { /* If after noon, set back to the AM. */
        call_in_hour -= 12;
    }

    pcall_in_params->call_in_start_date[0] = (ptm->tm_year & 0xff);     /* Call-in start YY */
    pcall_in_params->call_in_start_date[1] = (ptm->tm_mon + 1 & 0xff);  /* Call in start MM */
    pcall_in_params->call_in_start_date[2] = (ptm->tm_mday & 0xff);     /* Call in start DD */
    pcall_in_params->call_in_start_time[0] = call_in_hour;              /* Call-in start HH */
    pcall_in_params->call_in_start_time[1] = (ptm->tm_min & 0xff);      /* Call-in start MM */
    pcall_in_params->call_in_start_time[2] = (ptm->tm_sec & 0xff);      /* Call-in start SS */
    pcall_in_params->call_in_interval[0]   = 0;                         /* Call-in inteval DD */
    pcall_in_params->call_in_interval[1]   = 12;                        /* Call-in inteval HH */
    pcall_in_params->call_in_interval[2]   = 0;                         /* Call-in inteval MM */
    pcall_in_params->call_back_retry_time[0] = 59;                      /* Call-back retry time MM */
    pcall_in_params->call_back_retry_time[1] = 0;                       /* Call-back retry time SS */
    pcall_in_params->cdr_threshold = 4;                                 /* Indicates the number of CDRs that the terminal will store before automatically calling in to the Millennium Manager to upload them. (Range: 1-50) */
    pcall_in_params->unknown_timestamp[0] = (ptm->tm_year+1 & 0xff);    /* Unknown timestamp YY (2020) */
    pcall_in_params->unknown_timestamp[1] = (ptm->tm_mon + 1 & 0xff);   /* Unknown timestamp MM */
    pcall_in_params->unknown_timestamp[2] = (ptm->tm_mday & 0xff);      /* Unknown timestamp */
    pcall_in_params->unknown_timestamp[3] = 2;                          /* Unknown timestamp */
    pcall_in_params->unknown_timestamp[4] = 0;                          /* Unknown timestamp */
    pcall_in_params->unknown_timestamp[5] = 0;                          /* Unknown timestamp */
    pcall_in_params->unknown[0] = 16;
    pcall_in_params->unknown[1] = 14;

    printf("\tCall-in start date: %04d-%02d-%02d\n",
        ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
    printf("\tCall-in start time: %02d:%02d:%02d\n",
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    printf("\tCall-in interval:   %02dD:%02dH:%02dM\n",
        pcall_in_params->call_in_interval[0], pcall_in_params->call_in_interval[1], pcall_in_params->call_in_interval[2]);
    printf("\tCall-back retry:    %02dm:%02ds\n",
        pcall_in_params->call_back_retry_time[0], pcall_in_params->call_back_retry_time[1]);
    printf("\tCDR Threshold:      %d\n", pcall_in_params->cdr_threshold);

    *buffer = pbuffer;

    return 0;
}

int create_terminal_specific_directory(char *table_dir, char *terminal_id) {
    char dirname[80];
    int status = 0;
    errno = 0;

    snprintf(dirname, sizeof(dirname), "%s/%s", table_dir, terminal_id);

    status = mkdir(dirname, 0755);

    if(status != 0 && errno != EEXIST) {
        printf("Failed to create directory: %s\n", dirname);
        return (-1);
    }

    printf("Created directory: %s\n", dirname);
    return(status);
}

int update_terminal_cash_box_staus_table(mm_context_t *context, cashbox_status_univ_t *cashbox_status) {
    int status = 0;
    FILE *ostream = NULL;
    char filename[TABLE_PATH_MAX_LEN];

    snprintf(filename, sizeof(filename), "%s/%s/mm_table_26.bin", context->term_table_dir, context->terminal_id);
    printf("Saving CASH_BOX_STATUS_UNIV for terminal %s to %s\n", context->terminal_id, filename);

    /* Make sure the terminal-specific directory exists, create if needed. */
    create_terminal_specific_directory(context->term_table_dir, context->terminal_id);

    if(!(ostream = fopen(filename, "wb"))) {
        printf("Error writing %s\n", filename);
        return -1;
    }
    if (ostream != NULL) {
        fwrite(cashbox_status, sizeof(cashbox_status_univ_t), 1, ostream);
        fclose(ostream);
    }

    return(status);
}
