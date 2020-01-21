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


/* To Do List
 * ----------
 * Print full CDRs
 * Print Cash box status
 * Make custom LCD table generator.
 * Make custom Areacode table generator.
 */

#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <errno.h>   /* Error number definitions */
#include <time.h>    /* time_t, struct tm, time, gmtime */
#include <libgen.h>

#include "mm_manager.h"

/* Terminal Table Lists for Rev 1.3 and Rev 1.0 Control PCP */
uint8_t table_list_rev1_3[] = { 0x97, 0x96, 0x8a, 0x89, 0x88, 0x87, 0x86, 0x5d,
                              0x5c, 0x56, 0x55, 0x49, 0x48, 0x3e, 0x3a, 0x37,
                              0x32, 0x23, 0x22, 0x21, 0x20, 0x1f, 0x1e, 0x1d,
                              0x1a, 0x15, 0x0d, 0 };

uint8_t table_list_rev1_0[] = { 0x97, 0x96, 0x8a, 0x89, 0x88, 0x87, 0x86, 0x5d,
                              0x5c, 0x49, 0x3e, 0x3a, 0x37, 0x32, 0x23, 0x22,
                              0x21, 0x20, 0x1f, 0x1e, 0x1d, 0x1a, 0x15, 0x0d,
                              0 };

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
    int index;
    int c;

    time_t rawtime;
    struct tm *ptm;

    opterr = 0;
    mm_context.logstream = NULL;
    mm_context.use_modem = 0;
    mm_context.tx_seq = 0;
    mm_context.debuglevel = 0;
    mm_context.connected = 0;

    strncpy(mm_context.access_code, "2727378", 8); /* Default access code: "CRASERV" */

    printf("mm_manager v0.5 - (c) 2020, Howard M. Harte\n\n");

    index = 0;
    mm_context.ncc_number[0][0] = '\0';
    mm_context.ncc_number[1][0] = '\0';
    
    while ((c = getopt (argc, argv, "rvml:f:ha:n:")) != -1) {
        switch (c)
        {
            case 'h':
                printf("usage: %s [-vhm] [-f <filename>] [-l <logfile>] [-a <access_code>] [-n <ncc_number>]\n", basename(argv[0]));
                printf("\t-v verbose (multiple v's increase verbosity.)\n" \
                       "\t-f <filename> modem device or file\n" \
                       "\t-h this help.\n"
                       "\t-l <logfile> - log bytes transmitted to and received from the terminal.  Useful for debugging.\n" \
                       "\t-m use serial modem (specify device with -f)\n" \
                       "\t-n <Primary NCC Number> [-t <Secondary NCC Number>] - specify primary and optionally secondary NCC number.\n");
                       return(0);
                       break;
            case 'a':
                if (strlen(optarg) != 7) {
                    fprintf(stderr, "Option -a takes a 7-digit access code.\n");
                    return(-1);
                } else {
                    strncpy(mm_context.access_code, optarg, 8);
                    break;                    
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
            case 'n':
                if(index > 1) {
                    fprintf(stderr, "-n may only be specified twice.\n");
                    return(-1);
                }
                if ((strlen(optarg) < 7) || (strlen(optarg) > 15)) {
                    fprintf(stderr, "Option -n takes a 7- to 15-digit NCC number.\n");
                    return(-1);
                } else {
                    strncpy(mm_context.ncc_number[index], optarg, sizeof(mm_context.ncc_number[0]));
                    index++;                   
                }
                break;
            case '?':
                if (optopt == 'f' || optopt == 'l' || optopt == 'a' || optopt == 'n')
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

    printf("Using access code: %s\n", mm_context.access_code);

    if(strlen(mm_context.ncc_number[0]) >= 7) {
        printf("Using Primary NCC number: %s\n", mm_context.ncc_number[0]);
    } else {
        fprintf(stderr, "Error: -n <NCC Number> must be specified.\n");
        return(-1);
    }

    if(strlen(mm_context.ncc_number[1]) >= 7) {
        printf("Using Secondary NCC number: %s\n", mm_context.ncc_number[1]);
    }

    if (mm_context.use_modem == 0) {
        if(!(mm_context.bytestream = fopen(modem_dev, "r"))) {
            printf("Error opening input stream: %s\n", modem_dev);
            exit(-1);
        }
        mm_context.connected = 1;
    } else {    
        mm_context.fd = open_port(modem_dev);
        init_port(mm_context.fd);
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
        if(mm_context.use_modem == 1) {
            time ( &rawtime );
            ptm = localtime ( &rawtime );

            printf("\n\n%04d-%02d-%02d %2d:%02d:%02d: Disconnected.\n\n",
                ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

        }
    }

    return 0;
}


int receive_mm_table(mm_context_t *context, mm_table_t *table)
{
    mm_packet_t *pkt = &table->pkt;
    int i;
    uint8_t ack_payload[245] = {  0 };
    uint8_t *pack_payload = ack_payload;
    char serial_number[11];
    char terminal_version[12];
    uint8_t *ppayload;
    uint8_t cashbox_pending = 0;
    uint8_t table_download_pending = 0;
    uint8_t dont_send_reply = 0;
    uint8_t end_of_data = 0;

    if(receive_mm_packet(context, pkt) == 0) {
        context->rx_seq = pkt->hdr.flags & FLAG_SEQUENCE;
        
        ppayload = pkt->payload;

        if(pkt->payload_len >= PKT_TABLE_ID_OFFSET) {
            for (i=0; i < PKT_TABLE_ID_OFFSET; i++) {
                context->phone_number[i*2] = ((*ppayload & 0xf0) >> 4) + '0';
                context->phone_number[i*2+1] = (*ppayload++ & 0x0f) + '0';
            }
            context->phone_number[10] = '\0';

            if (context->debuglevel > 1) printf("\tReceived packet from phone# %s\n", context->phone_number);
        } else {
            table->table_id = 0;
            printf("Error: Received an ACK without expecting it!\n");
            return 0;
        }

        /* Acknowledge the received packet */
        send_mm_ack(context);

        while (ppayload < pkt->payload + pkt->payload_len) {

            table->table_id = *ppayload++;

            switch(table->table_id) {
                case TABLE_ID_TIME_SYNC_REQ: {
                    time_t rawtime;
                    struct tm *ptm;
                    uint8_t *timestamp;

                    printf("\tSeq: %d: Received time/date request\n", context->rx_seq);

                    *pack_payload++ = TABLE_ID_SET_DATE_TIME;
                    timestamp = pack_payload;
    
                    if (context->use_modem == 1) {
                        /* When using the modem, fill the current time.  If not using the modem, use
                        * a static time, so that results can be automatically checked.
                        */
                        time ( &rawtime );
                        ptm = localtime ( &rawtime );

                        *pack_payload++ = (ptm->tm_year & 0xff);     /* Fill current years since 1900 */
                        *pack_payload++ = (ptm->tm_mon+1 & 0xff);    /* Fill current month (1-12) */
                        *pack_payload++ = (ptm->tm_mday & 0xff);     /* Fill current day (0-31) */
                        *pack_payload++ = (ptm->tm_hour & 0xff);     /* Fill current hour (0-23) */
                        *pack_payload++ = (ptm->tm_min & 0xff);      /* Fill current minute (0-59) */
                        *pack_payload++ = (ptm->tm_sec & 0xff);      /* Fill current second (0-59) */
                        *pack_payload++ = 0;                         /* Hundredths of a second? */
                    } else {
                        *pack_payload++ = 119;  // Year
                        *pack_payload++ = 11;   // Month
                        *pack_payload++ = 9;    // Day
                        *pack_payload++ = 12;   // Hours
                        *pack_payload++ = 34;   // Minutes
                        *pack_payload++ = 56;   // Seconds
                        *pack_payload++ = 00;   // Hundredths
                    }
                    printf("Current day/time: %04d-%02d-%02d / %2d:%02d:%02d\n", timestamp[0]+1900,
                                                                                timestamp[1],
                                                                                timestamp[2],
                                                                                timestamp[3],
                                                                                timestamp[4],
                                                                                timestamp[5]);

                    end_of_data = 1;
                    break;
                }
                case TABLE_ID_ATTN_REQ_TABLE_UPD: {
                    uint8_t terminal_upd_reason;

                    terminal_upd_reason = *ppayload++;
                    printf("\tTerminal requests table update. Reason: 0x%02x [%s%s%s%s%s]\n\n", terminal_upd_reason,
                            terminal_upd_reason & TTBLREQ_CRAFT_FORCE_DL ? "Force Download, " : "",
                            terminal_upd_reason & TTBLREQ_CRAFT_INSTALL  ? "Install, " : "",
                            terminal_upd_reason & TTBLREQ_LOST_MEMORY    ? "Lost Memory, " : "",
                            terminal_upd_reason & TTBLREQ_PWR_LOST_ON_DL ? "Power Lost on Download, " : "",
                            terminal_upd_reason & TTBLREQ_CASHBOX_STATUS ? "Cashbox Status Request" : "");

                    if (terminal_upd_reason & TTBLREQ_CASHBOX_STATUS) {
                        cashbox_pending = 1;
                    }

                    table_download_pending = 1;
                    break;
                }
                case TABLE_ID_ALARM_MSG: {
                    mm_alarm_t *alarm = (mm_alarm_t *)ppayload;
                    uint8_t alarm_index;

                    ppayload += sizeof(mm_alarm_t);

                    *pack_payload++ = TABLE_ID_ALARM_ACK;
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

                    printf("\t\tAlarm: %04d-%02d-%02d %02d:%02d:%02d: Type: %d (0x%02x) - %s\n", 
                        alarm->timestamp[0] + 1900,
                        alarm->timestamp[1],
                        alarm->timestamp[2],
                        alarm->timestamp[3],
                        alarm->timestamp[4],
                        alarm->timestamp[5],
                        alarm->alarm_id, alarm->alarm_id,
                        alarm_type_str[alarm_index]);
                    break;
                }
                case TABLE_ID_MAINT_ACTION_REPORT: {
                    mm_maint_t *maint = (mm_maint_t *)ppayload;;
                    ppayload += sizeof(mm_maint_t);

                    printf("\t\tMaintenance Type: %d (0x%03x) Access PIN: %02x%02x%01x\n", 
                        maint->type, maint->type,
                        maint->access_pin[0], maint->access_pin[1], (maint->access_pin[2] & 0xF0) >> 4);

                    *pack_payload++ = TABLE_ID_MAINT_ACK;
                    *pack_payload++ = maint->type & 0xFF;
                    *pack_payload++ = (maint->type >> 8) & 0xFF;
                    break;
                }
                case TABLE_ID_CDR_POST_MSR15: {
                    cdr_record_t *cdr = (cdr_record_t *)ppayload;
                    ppayload += sizeof(cdr_record_t);

                    printf("\t\tCDR: %04d-%02d-%02d %02d:%02d:%02d: CDR Seq: %04d (0x%04x)\n", 
                        cdr->timestamp[0] + 1900,
                        cdr->timestamp[1],
                        cdr->timestamp[2],
                        cdr->timestamp[3],
                        cdr->timestamp[4],
                        cdr->timestamp[5],
                        cdr->seq, cdr->seq);
                    *pack_payload++ = TABLE_ID_CDR_ACK;
                    *pack_payload++ = cdr->seq & 0xFF;
                    *pack_payload++ = (cdr->seq >> 8) & 0xFF;
                    break;
                }
                case TABLE_ID_ATTN_REQ_CDR_UPLD: {
                    /* Not sure what the cdr_req_type is, just swallow it. */
                    uint8_t cdr_req_type = *ppayload++;
                    printf("Seq: %d: TABLE_ID_ATTN_REQ_CDR_UPLD, cdr_req_type=%02x (0x%02x)\n", context->tx_seq, cdr_req_type, cdr_req_type);
                    
                    *pack_payload++ = TABLE_ID_XMIT_TERMINAL_DATA;
                    break;
                }
                case TABLE_ID_CASHBOX_COLL_UNIV: {
                    cashbox_coll_univ_t *cashbox_coll_univ = (cashbox_coll_univ_t *)ppayload;
                    printf("\tSeq: %d: TABLE_ID_CASHBOX_COLL_UNIV.\n", context->tx_seq);

                    ppayload += sizeof(cashbox_coll_univ_t);

                    printf("\t\t%04d-%02d-%02d %02d:%02d:%02d: ",
                            cashbox_coll_univ->timestamp[0] + 1900,
                            cashbox_coll_univ->timestamp[1],
                            cashbox_coll_univ->timestamp[2],
                            cashbox_coll_univ->timestamp[3],
                            cashbox_coll_univ->timestamp[4],
                            cashbox_coll_univ->timestamp[5]);
                    
                    dump_hex(cashbox_coll_univ->cash, sizeof(cashbox_coll_univ->cash));
                    break;
                }
                case TABLE_ID_TERM_STATUS_MSG: {
                    term_status_t *term_status = (term_status_t *)ppayload;
                    uint8_t serial_number[11];
                    uint64_t term_status_word;
                    int i;

                    ppayload += sizeof(term_status_t);

                    for (i=0; i < 5; i++) {
                        serial_number[i*2] = ((term_status->serialnum[i] & 0xf0) >> 4) + '0';
                        serial_number[i*2+1] = (term_status->serialnum[i] & 0x0f) + '0';
                    }

                    serial_number[10] = '\0';

                    term_status_word  = term_status->status[0];
                    term_status_word |= term_status->status[1] << 8;
                    term_status_word |= term_status->status[2] << 16;
                    term_status_word |= term_status->status[3] << 24;
                    term_status_word |= (uint64_t)(term_status->status[4]) << 32;

                    printf("\tTerminal serial number %s, Terminal Status Word: 0x%010llx\n",
                        serial_number, term_status_word);

                    /* Iterate over all the terminal status bits and display a message for any flags set. */
                    for (i = 0; term_status_word != 0; i++) {
                        if (term_status_word & 1) {
                            printf("Terminal Status: %s\n", alarm_type_str[i]);
                        }
                        term_status_word >>= 1;
                    }           
                    break;
                }
                case TABLE_ID_SW_VERSION_MSG: {
                    term_version_t *term_version = (term_version_t *)ppayload;

                    char control_rom_edition[sizeof(term_version->control_rom_edition) + 1] = { 0 };
                    char control_version[sizeof(term_version->control_version) + 1] = { 0 };
                    char telephony_rom_edition[sizeof(term_version->telephony_rom_edition) + 1] = { 0 };
                    char telephony_version[sizeof(term_version->telephony_version) + 1] = { 0 };
                    memcpy(control_rom_edition, term_version->control_rom_edition, sizeof(term_version->control_rom_edition));
                    memcpy(control_version, term_version->control_version, sizeof(term_version->control_version));
                    memcpy(telephony_rom_edition, term_version->telephony_rom_edition, sizeof(term_version->telephony_rom_edition));
                    memcpy(telephony_version, term_version->telephony_version, sizeof(term_version->telephony_version));

                    ppayload += sizeof(term_version_t);

                    if (strcmp(control_version, "V1.0") == 0) {
                        context->phone_rev = 10;
                    } else if (strcmp(control_version, "V1.3") == 0) {
                        context->phone_rev = 13;
                    } else {
                        printf("Error: Unknown control version %s, defaulting to tables for V1.0.\n", control_version);
                        context->phone_rev = 10;
                    }

                    printf("\t\t             Terminal Type: %02d (0x%02x)\n", term_version->term_type, term_version->term_type);
                    printf("\t\t       Control ROM Edition: %s\n", control_rom_edition);
                    printf("\t\t           Control Version: %s\n", control_version);
                    printf("\t\t     Telephony ROM Edition: %s\n", telephony_rom_edition);
                    printf("\t\t         Telephony Version: %s\n", telephony_version);
                    printf("\t\tValidator Hardware Version: %c%c\n", term_version->validator_hw_ver[0], term_version->validator_hw_ver[1]);
                    printf("\t\tValidator Software Version: %c%c\n", term_version->validator_sw_ver[0], term_version->validator_sw_ver[1]);

                    break;
                }
                case TABLE_ID_CASH_BOX_STATUS: {
                    cashbox_status_univ_t *cashbox_status = (cashbox_status_univ_t *)ppayload;
                    ppayload += sizeof(cashbox_status_univ_t);

                    printf("Cashbox status:\n");
                    printf("\t\t%04d-%02d-%02d %02d:%02d:%02d: ",
                            cashbox_status->timestamp[0] + 1900,
                            cashbox_status->timestamp[1],
                            cashbox_status->timestamp[2],
                            cashbox_status->timestamp[3],
                            cashbox_status->timestamp[4],
                            cashbox_status->timestamp[5]);
                    break;
                }
                case TABLE_ID_PERF_STATS_RECORD: {
                    perf_stats_record_t *perf_stats = (perf_stats_record_t *)ppayload;
                    ppayload += sizeof(perf_stats_record_t);

                    printf("\t\tPerformance Statistics Record: From %04d-%02d-%02d %02d:%02d:%02d, to: %04d-%02d-%02d %02d:%02d:%02d:\n",
                            perf_stats->timestamp[0] + 1900,
                            perf_stats->timestamp[1],
                            perf_stats->timestamp[2],
                            perf_stats->timestamp[3],
                            perf_stats->timestamp[4],
                            perf_stats->timestamp[5],
                            perf_stats->timestamp2[0] + 1900,
                            perf_stats->timestamp2[1],
                            perf_stats->timestamp2[2],
                            perf_stats->timestamp2[3],
                            perf_stats->timestamp2[4],
                            perf_stats->timestamp2[5]);
                    break;
                }
                case TABLE_ID_TERM_CALL_IN_MSG: {
                    printf("Seq: %d: TABLE_ID_TERM_CALL_IN_MSG.\n", context->tx_seq);
                    *pack_payload++ = TABLE_ID_XMIT_TERMINAL_DATA;
                    break;
                }
                case TABLE_ID_TERM_CALL_BACK_MSG: {
                    printf("Seq: %d: TABLE_ID_TERM_CALL_BACK_MSG.\n", context->tx_seq);
                    *pack_payload++ = TABLE_ID_XMIT_TERMINAL_DATA;
                    break;
                }
                case TABLE_ID_CARR_CALL_STATS:
                {
                    carr_call_stats_t *carr_stats = (carr_call_stats_t *)ppayload;
                    ppayload += sizeof(carr_call_stats_t);
                    printf("Seq: %d: TABLE_ID_CARR_CALL_STATS.\n", context->tx_seq);
                    break;
                }
                case TABLE_ID_EXP_CAR_CALL_STATS: {
                    exp_carr_call_stats_t *carr_stats = (exp_carr_call_stats_t *)ppayload;
                    ppayload += sizeof(exp_carr_call_stats_t);
                    printf("Seq: %d: TABLE_ID_EXP_CARR_CALL_STATS.\n", context->tx_seq);
                    *pack_payload++ = TABLE_ID_END_OF_DATA_MSG;
                    break;
                }
                case TABLE_ID_SUMMARY_CALL_STATS: {
                    summary_call_stats_t *summary_call_stats = (summary_call_stats_t *)ppayload;
                    ppayload += sizeof(summary_call_stats_t);

                    printf("Summary call stats:\n");
                    dump_hex((uint8_t *)summary_call_stats, sizeof(summary_call_stats));

                    break;
                }
                case TABLE_ID_RATE_REQ_UNIV: {
                    char phone_number[21] = { 0 };
                    uint8_t rate_response[] = {TABLE_ID_RATE_RESP_UNIV, 0x28, 0x00, 0xff, 0xff, 0x32, 0x00, 0xff, 0xff, 0x00, 0x00};
                    rate_request_t *rate_request = (rate_request_t *)ppayload;
                    ppayload += sizeof(rate_request_t);


                    for (i=0; i < 10; i++) {
                        phone_number[i*2] = ((rate_request->phone_number[i] & 0xf0) >> 4) + '0';
                        if (phone_number[i*2] == '>') break;
                        phone_number[i*2+1] = (rate_request->phone_number[i] & 0x0f) + '0';
                        if (phone_number[i*2+1] == '>') break;
                    }
                    printf("\t\tRate request: %04d-%02d-%02d %02d:%02d:%02d: Phone number: %s, seq=%d, %d,%d,%d,%d,%d,%d.\n",
                            rate_request->timestamp[0] + 1900,
                            rate_request->timestamp[1],
                            rate_request->timestamp[2],
                            rate_request->timestamp[3],
                            rate_request->timestamp[4],
                            rate_request->timestamp[5],
                            phone_number,
                            rate_request->seq,
                            rate_request->pad[0],
                            rate_request->pad[1],
                            rate_request->pad[2],
                            rate_request->pad[3],
                            rate_request->pad[4],
                            rate_request->pad[5]);
                    
                    memcpy(pack_payload, rate_response, sizeof(rate_response));
                    pack_payload += sizeof(rate_response);
                    break;
                }
                case TABLE_ID_END_OF_DATA_MSG:
                    printf("Seq: %d: TABLE_ID_END_OF_DATA_MSG.\n", context->tx_seq);
                    *pack_payload++ = TABLE_ID_END_OF_DATA_MSG;
                    break;
                case TABLE_ID_TABLE_UPDATE_ACK:
                    printf("Seq: %d: TABLE_ID_TABLE_UPDATE_ACK for table 0x%02x.\n", context->tx_seq, *ppayload);
                    ppayload++;
                    *pack_payload++ = TABLE_ID_XMIT_TERMINAL_DATA;
                    break;
                default:
                    printf("Unhandled table %d (0x%02x)", table->table_id, table->table_id);
                    send_mm_ack(context);
                    break;
            }
        }

        if (cashbox_pending == 1) {
            uint8_t cashbox_status[] = {
                TABLE_ID_CASH_BOX_STATUS, 0x5a, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00,
                00, 00, 0x32, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
                00, 00, 00, 00, 02, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
                00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };

            printf("\tSeq %d: Send TABLE_ID_CASH_BOX_STATUS table as requested by terminal.\n", context->tx_seq);
            memcpy(pack_payload, cashbox_status, sizeof(cashbox_status));
            pack_payload += sizeof(cashbox_status);
        }

        if (dont_send_reply == 0) {
            if (pack_payload - ack_payload == 0) {
                *pack_payload++ = TABLE_ID_END_OF_DATA_MSG;
            }
            send_mm_table(context, ack_payload, (pack_payload - ack_payload), end_of_data);
        }
        if (table_download_pending == 1) {
            mm_download_tables(context);
        }

        return 0;
    } else {
        return -1;
    }
}


int mm_download_tables(mm_context_t *context)
{
    uint8_t table_data = TABLE_ID_TERM_TABLE_DATA_UPD;
    int table_index;
    int table_len;
    uint8_t *table_buffer;
    uint8_t *table_list = table_list_rev1_3;

    /* Rev 1 PCP does not accept table 0x048, 0x55, 0x56 */
    if (context->phone_rev == 10) table_list = table_list_rev1_0;

    send_mm_table(context, &table_data, 1, 0);

    for (table_index = 0; table_list[table_index] > 0; table_index++) {
        load_mm_table(table_list[table_index], &table_buffer, &table_len);

        switch(table_list[table_index]) {
            case TABLE_ID_INST_SERV_PARAM:
                rewrite_instserv_parameters(context, table_buffer, table_len);
                break;
            case TABLE_ID_TERM_ACCESS_PARAM:
                rewrite_term_access_parameters(context, table_buffer, table_len);
                break;
            default:
                break;
        }
        send_mm_table(context, table_buffer, table_len, 0);
        
        /* For all tables except END_OF_DATA, expect a table ACK. */
        if (table_list[table_index] != TABLE_ID_END_OF_DATA_MSG) {
            wait_for_table_ack(context, table_buffer[0]);
        }
        free(table_buffer);
    }

    return 0;
}


int send_mm_table(mm_context_t *context, uint8_t *payload, int len, int end_of_data)
{
    int i;
    mm_packet_t pkt;
    int bytes_remaining;
    int chunk_len;
    uint8_t *p = payload;  
    uint8_t table_id;
    uint8_t end_of_data_msg = TABLE_ID_END_OF_DATA_MSG;

    table_id = payload[0];
    bytes_remaining = len;
    printf("\nSending Table ID %d (0x%02x)...\n", table_id, table_id);
    while (bytes_remaining > 0) {
        if (bytes_remaining > 245) {
            chunk_len = 245;
        } else {
            chunk_len = bytes_remaining;
        }

        send_mm_packet(context, p, chunk_len);

        if (wait_for_mm_ack(context) != 0) return -1;
        p += chunk_len;
        bytes_remaining -= chunk_len;
        printf("Table %d progress: (%3ld%%) - %ld / %d \n", table_id, ((p - payload) * 100) / len, p - payload, len);
    }

    if (context->debuglevel > 1) printf("Sending end of data message.\n");

    if(end_of_data != 0) {
        send_mm_packet(context, &end_of_data_msg, sizeof(end_of_data_msg));
        if (wait_for_mm_ack(context) != 0) return -1;
    }

    return 0;
}


int wait_for_table_ack(mm_context_t *context, uint8_t table_id)
{
    mm_packet_t packet;
    mm_packet_t *pkt = &packet;
    int i;

    if (context->debuglevel > 1) printf("Waiting for ACK for table %d (0x%02x)\n", table_id, table_id);
    
    if(receive_mm_packet(context, pkt) == 0) {
        context->rx_seq = pkt->hdr.flags & FLAG_SEQUENCE;
        
        if(pkt->payload_len >= PKT_TABLE_ID_OFFSET) {
            for (i=0; i < PKT_TABLE_ID_OFFSET; i++) {
                context->phone_number[i*2] = ((pkt->payload[i] & 0xf0) >> 4) + '0';
                context->phone_number[i*2+1] = (pkt->payload[i] & 0x0f) + '0';
            }
            context->phone_number[10] = '\0';
            if (context->debuglevel > 1) printf("Received packet from phone# %s\n", context->phone_number);

            if((pkt->payload[PKT_TABLE_ID_OFFSET] == TABLE_ID_TABLE_UPDATE_ACK) && 
               (pkt->payload[6] == table_id)) {
                   if (context->debuglevel > 0) printf("Seq: %d: Received ACK for table %d (0x%02x)\n", context->rx_seq, table_id, table_id);
                   send_mm_ack(context);
               } else {
                   printf("Error: Did not receive ACK for table id %d\n", table_id);
                   return (-1);
               }

        }
    }
    return 0;
}


int load_mm_table(uint8_t table_id, uint8_t **buffer, int *len)
{
    FILE *stream;
    char fname[80];
    uint8_t c;
    long size;
    uint8_t *bufp;

    snprintf(fname, sizeof(fname), "./tables/mm_table_%02x.bin", table_id);
    if(!(stream = fopen(fname, "rb"))) {
        printf("Error loading %s\n", fname);
        exit(-1);
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

    printf("\nLoaded table ID %d (0x%02x) from %s (%d bytes).\n", table_id, table_id, fname, *len - 1);
    fclose(stream);

    return 0;
}

#define ACCESS_CODE_OFFSET 1
int rewrite_instserv_parameters(mm_context_t *context, uint8_t *table_buffer, int table_len)
{
    int i;

    if (strlen(context->access_code) != 7) {
        printf("Error: Access Code must be 7-digits\n");
    }

    printf("\nRewriting Installation / Servicing Parameters table:\n" \
           "\t  Access Code: %s", context->access_code);

    // Rewrite table with our Access Code
    for (i = 0; i < (strlen(context->access_code)); i++) {
        if (i % 2 == 0) {
            table_buffer[ACCESS_CODE_OFFSET + (i >> 1)]  = (context->access_code[i] - '0') << 4;
        } else {
            table_buffer[ACCESS_CODE_OFFSET + (i >> 1)] |= (context->access_code[i] - '0');
        }
    }
    table_buffer[ACCESS_CODE_OFFSET + 3] |= 0x0e;   /* Terminate the Access Code with 0xe */

    return 0;
}

#define TERM_ID_OFFSET 1
#define PRI_NCC_OFFSET 6
#define SEC_NCC_OFFSET 16

int rewrite_term_access_parameters(mm_context_t *context, uint8_t *table_buffer, int table_len)
{
    int i;

    /* Blank out the primary NCC number. */
    for (i=0; i <= 10; i++) {
        table_buffer[SEC_NCC_OFFSET + i] = 0;
    }

    /* Blank out the secondary NCC number. */
    for (i=0; i <= 10; i++) {
        table_buffer[SEC_NCC_OFFSET + i] = 0;
    }

    printf("\nRewriting Terminal Access Parameters table:\n" \
           "\t  Terminal ID:  %s\n", context->phone_number);

    // Rewrite table with our Terminal ID (phone number)
    for (i = 0; i < PKT_TABLE_ID_OFFSET; i++) {
        table_buffer[TERM_ID_OFFSET + i]  = (context->phone_number[i * 2    ] - '0') << 4;
        table_buffer[TERM_ID_OFFSET + i] |= (context->phone_number[i * 2 + 1] - '0');
    }

    // Rewrite table with Primary NCC phone number
    printf("\t  Primary NCC: %s\n", context->ncc_number[0]);
    for (i = 0; i < (strlen(context->ncc_number[0])); i++) {
        if (i % 2 == 0) {
            table_buffer[PRI_NCC_OFFSET + (i >> 1)]  = (context->ncc_number[0][i] - '0') << 4;
        } else {
            table_buffer[PRI_NCC_OFFSET + (i >> 1)] |= (context->ncc_number[0][i] - '0');
        }
    }

    if (strlen(context->ncc_number[1]) == 11) {
        // Rewrite table with Secondary NCC phone number
        printf("\tSecondary NCC: %s\n\n", context->ncc_number[1]);
        for (i = 0; i < strlen(context->ncc_number[1]); i++) {
            if (i % 2 == 0) {
                table_buffer[SEC_NCC_OFFSET + (i / 2)]  = (context->ncc_number[1][i] - '0') << 4;
            } else {
                table_buffer[SEC_NCC_OFFSET + (i / 2)] |= (context->ncc_number[1][i] - '0');
            }
        }
    }

    return 0;
}



