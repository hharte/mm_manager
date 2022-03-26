/*
 * This is a "Manager" for the Nortel Millennium payhone.
 *
 * It can provision a Nortel Millennium payphone with Rev 1.0 or 1.3
 * Control PCP.  CDRs, Alarms, and Maintenance Reports can also be
 * retieved.
 *
 * www.github.com/hharte/mm_manager
 *
 * (c) 2020-2022, Howard M. Harte
 */

#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>  /* String function definitions */
#ifndef _WIN32
#include <unistd.h>  /* UNIX standard function definitions */
#include <libgen.h>
#else
#include <direct.h>
#include "third-party/getopt.h"
#endif
#include <errno.h>   /* Error number definitions */
#include <time.h>    /* time_t, struct tm, time, gmtime */

#include <sys/stat.h>

#include "mm_manager.h"
#include "mm_serial.h"

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
    DLOG_MT_CARRIER_TABLE,      /* Required */
    DLOG_MT_CARD_TABLE,         /* Required */
    DLOG_MT_SCARD_PARM_TABLE,   /* Required */
    DLOG_MT_CALL_SCREEN_LIST,   /* Required */
    DLOG_MT_VIS_PROPTS_L2,      /* 1.3 only */
    DLOG_MT_VIS_PROPTS_L1,      /* 1.3 only */
    DLOG_MT_RATE_TABLE,         /* Required */
    DLOG_MT_SPARE_TABLE,        /* 1.3 only */
    DLOG_MT_NUM_PLAN_TABLE,     /* Required */
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
    DLOG_MT_CARRIER_TABLE,      /* Required */
    DLOG_MT_CARD_TABLE,         /* Required */
    DLOG_MT_SCARD_PARM_TABLE,   /* Required */
    DLOG_MT_CALL_SCREEN_LIST,   /* Required */
    DLOG_MT_RATE_TABLE,         /* Required */
    DLOG_MT_NUM_PLAN_TABLE,     /* Required */
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
    DLOG_MT_CARRIER_TABLE,      /* Required */
    DLOG_MT_CARD_TABLE,         /* Required */
    DLOG_MT_SCARD_PARM_TABLE,   /* Required */
    DLOG_MT_CALL_SCREEN_LIST,   /* Required */
    DLOG_MT_RATE_TABLE,         /* Required */
    DLOG_MT_SPARE_TABLE,
    DLOG_MT_NUM_PLAN_TABLE,     /* Required */
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

const char *stats_call_type_str_lut[4] = {
    "Local        ",
    "Inter-LATA   ",
    "Intra-LATA   ",
    "International"
};

const char *stats_to_str_lut[29] = {
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

const char *TCALSTE_stats_to_str_lut[16] = {
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

const char *TPERFST_stats_to_str_lut[43] = {
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

const char cmdline_options[] = "rvmb:c:d:l:f:ha:n:st:q";

#if _WIN32
char *basename(char *path)
{
    char fname[20] = { 0 };

    _splitpath(path, NULL, NULL, fname, NULL);

    snprintf(path, sizeof(fname), "%s", fname);
    return (path);
}
#endif /* _WIN32 */

int main(int argc, char* argv[])
{
    mm_context_t *mm_context;
    mm_table_t mm_table;
    char *modem_dev = NULL;
    char *table_dir = NULL;
    int index;
    int c;
    uint8_t *instsv_table_buffer;
    int table_len;
    int baudrate = 19200;
    char access_code_str[8];
    int quiet = 0;

    time_t rawtime;
    struct tm *ptm;

    opterr = 0;

    mm_context = calloc(1, sizeof(mm_context_t));
    if (mm_context == NULL) {
        printf("Error: failed to allocate %d bytes.\n", (int)sizeof(mm_context_t));
        return (-1);
    }

    mm_context->logstream = NULL;
    mm_context->cdr_stream = NULL;
    mm_context->use_modem = 0;
    mm_context->tx_seq = 0;
    mm_context->debuglevel = 0;
    mm_context->connected = 0;
    mm_context->terminal_id[0] = '\0';
    mm_context->trans_data_in_progress = 0;

    index = 0;
    mm_context->ncc_number[0][0] = '\0';
    mm_context->ncc_number[1][0] = '\0';

    strcpy(mm_context->default_table_dir, "tables/default");
    strcpy(mm_context->term_table_dir, "tables");
    mm_context->minimal_table_set = 0;

    /* Parse command line to get -q (quiet) option. */
    while ((c = getopt (argc, argv, cmdline_options)) != -1) {
        switch (c)
        {
            case 'q':
                quiet = 1;
                break;
            default:
                break;
        }
    }

    if (quiet == 0) {
        printf("mm_manager v0.6 [%s] - (c) 2020-2022, Howard M. Harte\n\n", VERSION);
    }

    /* Parse command line again to get the rest of the options. */
    optind = 1;
    while ((c = getopt (argc, argv, cmdline_options)) != -1) {
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
            {
                char instsv_fname[TABLE_PATH_MAX_LEN] = { 0 };
                snprintf(instsv_fname, sizeof(instsv_fname), "%s/mm_table_1f.bin", mm_context->default_table_dir);

                if (strnlen(optarg, 7) != 7) {
                    fprintf(stderr, "Option -a takes a 7-digit access code.\n");
                    return(-1);
                }

                if (load_mm_table(mm_context, DLOG_MT_INSTALL_PARAMS, &instsv_table_buffer, &table_len)) {
                    fprintf (stderr, "Error reading install parameters from %s.\n", instsv_fname);
                    return -1;
                }
                memcpy(&mm_context->instsv, instsv_table_buffer+1, sizeof(dlog_mt_install_params_t));
                free(instsv_table_buffer);

                if (rewrite_instserv_parameters(optarg, &mm_context->instsv, instsv_fname)) {
                    printf("Error updating INSTSV parameters\n");
                    return (-1);
                }
                break;
            }
            case 'c':
                if (!(mm_context->cdr_stream = fopen(optarg, "a+"))) {
                    (void)fprintf(stderr,
                        "mm_manager: %s: %s\n", optarg, strerror(errno));

                    if (mm_context != NULL) free(mm_context);
                    exit(1);
                }
                fseek(mm_context->cdr_stream, 0, SEEK_END);
                if (ftell(mm_context->cdr_stream) == 0) {
                    printf("Creating CDR file '%s'\n", optarg);
                    fprintf(mm_context->cdr_stream, "TERMINAL_ID,SEQ,TIMESTAMP,DURATION,TYPE,DIALED_NUM,CARD,REQUESTED,COLLECTED,CARRIER,RATE\n");
                    fflush(mm_context->cdr_stream);
                } else {
                    printf("Appending to existing CDR file '%s'\n", optarg);
                }
                break;
            case 'v':
                mm_context->debuglevel++;
                break;
            case 'f':
                modem_dev = optarg;
                break;
            case 'l':
                if (!(mm_context->logstream = fopen(optarg, "w"))) {
                    (void)fprintf(stderr,
                        "mm_manager: %s: %s\n", optarg, strerror(errno));
                    if (mm_context != NULL) free(mm_context);
                    exit(1);
                }
                break;
            case 'm':
                mm_context->use_modem = 1;
                break;
            case 'b':
                baudrate = atoi(optarg);
                break;
            case 'n':
                if(index > 1) {
                    fprintf(stderr, "-n may only be specified twice.\n");
                    return(-1);
                }
                if ((strnlen(optarg, 16) < 1) || (strnlen(optarg, 16) > 15)) {
                    fprintf(stderr, "Option -n takes a 1- to 15-digit NCC number.\n");
                    return(-1);
                } else {
                    snprintf(mm_context->ncc_number[index], sizeof(mm_context->ncc_number[0]), "%s", optarg);
                    index++;
                }
                break;
            case 's':
                printf("NOTE: Using minimum required table list for download.\n");
                mm_context->minimal_table_set = 1;
                break;
            case 'd':
                snprintf(mm_context->default_table_dir, sizeof(mm_context->default_table_dir), "%s", optarg);
                break;
            case 't':
                snprintf(mm_context->term_table_dir, sizeof(mm_context->term_table_dir), "%s", optarg);
                break;
            case 'q':
                break;
            case '?':
                if (optopt == 'f' || optopt == 'l' || optopt == 'a' || optopt == 'n' || optopt == 'b')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                return 1;
                break;
            default:
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                return(-1);
        }
    }

    for (index = optind; index < argc; index++) {
        printf ("Superfluous non-option argument '%s' ignored.\n", argv[index]);
    }

    printf("Default Table directory: %s\n", mm_context->default_table_dir);
    printf("Terminal-specific Table directory: %s/<terminal_id>\n", mm_context->term_table_dir);

    if (load_mm_table(mm_context, DLOG_MT_INSTALL_PARAMS, &instsv_table_buffer, &table_len)) {
        fprintf (stderr, "Error reading install parameters from %s/mm_table_1f.bin.\n", mm_context->default_table_dir);
        return -1;
    }
    memcpy(&mm_context->instsv, instsv_table_buffer+1, sizeof(dlog_mt_install_params_t));
    free(instsv_table_buffer);

    printf("Using access code: %s\n", phone_num_to_string(access_code_str, sizeof(access_code_str), mm_context->instsv.access_code, sizeof(mm_context->instsv.access_code)));
    printf("Manager Inter-packet Tx gap: %dms.\n", mm_context->instsv.rx_packet_gap * 10);

    if(strnlen(mm_context->ncc_number[0], sizeof(mm_context->ncc_number[0])) >= 1) {
        printf("Using Primary NCC number: %s\n", mm_context->ncc_number[0]);

        if(strnlen(mm_context->ncc_number[1], sizeof(mm_context->ncc_number[0])) == 0) {
            snprintf(mm_context->ncc_number[1], sizeof(mm_context->ncc_number[1]), "%s", mm_context->ncc_number[0]);
        }

        printf("Using Secondary NCC number: %s\n", mm_context->ncc_number[1]);
    } else if (mm_context->use_modem == 1) {
        fprintf(stderr, "Error: -n <NCC Number> must be specified.\n");
        return(-1);
    }

    if (mm_context->use_modem == 0) {
        if (modem_dev == NULL) {
            (void)fprintf(stderr, "mm_manager: -f <filename> must be specified.\n");
            if (mm_context != NULL) free(mm_context);
            exit(1);
        }

        if(!(mm_context->bytestream = fopen(modem_dev, "r"))) {
            printf("Error opening input stream: %s\n", modem_dev);
            if (mm_context != NULL) free(mm_context);
            exit(-1);
        }
        mm_context->connected = 1;
    } else {
        int status;

        if (baudrate < 1200) {
            printf("Error: baud rate must be 1200 bps or faster.\n");
            return(-1);
        }
        printf("Baud Rate: %d\n", baudrate);

        mm_context->fd = open_serial(modem_dev);
        if (mm_context->fd == -1) {
            printf("Unable to open modem: %s.", modem_dev);
            return (-1);
        }

        init_serial(mm_context->fd, baudrate);
        status = init_modem(mm_context->fd);
        if (status == 0) {
            printf("Modem initialized.\n");
        } else {
            printf("Error initializing modem.\n");
            close_serial(mm_context->fd);
            return (-1);
        }
    }

    mm_context->cdr_ack_buffer_len = 0;

    while(1) {
        if(mm_context->use_modem == 1) {
            printf("Waiting for call from terminal...\n");
            if(wait_for_modem_response(mm_context->fd, "CONNECT", 1000) == 0) {

                mm_context->tx_seq = 0;
                time ( &rawtime );
                ptm = localtime ( &rawtime );

                printf("%04d-%02d-%02d %2d:%02d:%02d: Connected!\n\n",
                    ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

                mm_context->connected = 1;
                mm_context->cdr_ack_buffer_len = 0;
            } else {
                printf("Timed out waiting for connect, retrying...\n");
                continue;
            }
        }

        while ((receive_mm_table(mm_context, &mm_table) == 0) && (mm_context->connected == 1)) {
        }
        if (mm_context->use_modem == 1) {
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

static int append_to_cdr_ack_buffer(mm_context_t *context, uint8_t *buffer, uint8_t length)
{
    if ((size_t)context->cdr_ack_buffer_len + length > sizeof(context->cdr_ack_buffer)) {
        printf("ERROR: %s: cdr_ack_buffer_len exceeded.\n", __FUNCTION__);
        return (-1);
    }

    memcpy(&context->cdr_ack_buffer[context->cdr_ack_buffer_len], buffer, length);
    context->cdr_ack_buffer_len += length;

    return(0);
}

int receive_mm_table(mm_context_t *context, mm_table_t *table)
{
    mm_packet_t *pkt = &table->pkt;
    uint8_t ack_payload[PKT_TABLE_DATA_LEN_MAX] = { 0 };
    uint8_t *pack_payload = ack_payload;
    char timestamp_str[20];
    char timestamp2_str[20];
    uint8_t *ppayload;
    uint8_t cashbox_pending = 0;
    uint8_t dont_send_reply = 0;
    uint8_t table_download_pending = 0;
    uint8_t end_of_data = 0;
    uint8_t status;

    if ((status = receive_mm_packet(context, pkt)) != 0) {
        if (context->debuglevel > 2) print_mm_packet(RX, pkt);
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
                uint8_t cdr_ack_buf[3] = { 0 };

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

                cdr_ack_buf[0] = DLOG_MT_CDR_DETAILS_ACK;
                cdr_ack_buf[1] = cdr->seq & 0xFF;
                cdr_ack_buf[2] = (cdr->seq >> 8) & 0xFF;

                /* If terminal is transferring multiple tables, queue the CDR response for later, after receiving DLOG_MT_END_DATA */
                if (context->trans_data_in_progress == 1) {
                    append_to_cdr_ack_buffer(context, cdr_ack_buf, sizeof(cdr_ack_buf));
                    dont_send_reply = 1;
                } else {
                    /* If receiving a CDR as part of a credit card auth, etc, send the CDR ack immediately. */
                    memcpy(pack_payload, cdr_ack_buf, sizeof(cdr_ack_buf));
                    pack_payload += sizeof(cdr_ack_buf);
                }
                break;
            }
            case DLOG_MT_ATN_REQ_CDR_UPL: {
                /* Not sure what the cdr_req_type is, just swallow it. */
                uint8_t cdr_req_type = *ppayload++;
                printf("\t\tDLOG_MT_ATN_REQ_CDR_UPL, cdr_req_type=%02x (0x%02x)\n", cdr_req_type, cdr_req_type);

                *pack_payload++ = DLOG_MT_TRANS_DATA;
                context->trans_data_in_progress = 1;
                break;
            }
            case DLOG_MT_CASH_BOX_COLLECTION: {
                dlog_mt_cash_box_collection_t *cash_box_collection = (dlog_mt_cash_box_collection_t *)ppayload;

                ppayload += sizeof(dlog_mt_cash_box_collection_t);

                printf("\t\tCashbox Collection: %s: Total: $%3.2f (%3d%% full): CA N:%d D:%d Q:%d $:%d - US N:%d D:%d Q:%d $:%d\n",
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
                *pack_payload++ = DLOG_MT_END_DATA;
                break;
            }
            case DLOG_MT_TERM_STATUS: {
                dlog_mt_term_status_t *dlog_mt_term_status = (dlog_mt_term_status_t *)ppayload;
                uint8_t serial_number[11] = { 0 };
                unsigned long long dlog_mt_term_status_word;
                int i;

                ppayload += sizeof(dlog_mt_term_status_t);

                for (i=0; i < 5; i++) {
                    serial_number[i*2] = ((dlog_mt_term_status->serialnum[i] & 0xf0) >> 4) + '0';
                    serial_number[i*2+1] = (dlog_mt_term_status->serialnum[i] & 0x0f) + '0';
                }

                serial_number[10] = '\0';

                dlog_mt_term_status_word  = dlog_mt_term_status->status[0];
                dlog_mt_term_status_word |= (uint64_t)(dlog_mt_term_status->status[1]) << 8;
                dlog_mt_term_status_word |= (uint64_t)(dlog_mt_term_status->status[2]) << 16;
                dlog_mt_term_status_word |= (uint64_t)(dlog_mt_term_status->status[3]) << 24;
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

                for (int i = 0; i < ((sizeof(perf_stats->stats) / sizeof(uint16_t))); i++) {
                    if (perf_stats->stats[i] > 0) printf("[%2d] %27s: %5d\n", i, TPERFST_stats_to_str_lut[i], perf_stats->stats[i]);
                }
                dont_send_reply = 1;
                break;
            }
            case DLOG_MT_CALL_IN:
            case DLOG_MT_CALL_BACK: {
                *pack_payload++ = DLOG_MT_TRANS_DATA;
                context->trans_data_in_progress = 1;
                break;
            }
            case DLOG_MT_CARRIER_CALL_STATS:
            {
                dlog_mt_carrier_call_stats_t *carr_stats = (dlog_mt_carrier_call_stats_t *)ppayload;
                ppayload += sizeof(dlog_mt_carrier_call_stats_t);
                printf("\t\tCarrier Call Statistics Record: From: %s, to: %s:\n",
                        timestamp_to_string(carr_stats->timestamp, timestamp_str, sizeof(timestamp_str)),
                        timestamp_to_string(carr_stats->timestamp2, timestamp2_str, sizeof(timestamp2_str)));

                for (int i = 0; i < 3; i++) {
                    carrier_stats_entry_t *pcarr_stats_entry = &carr_stats->carrier_stats[i];
                    uint32_t k = 0;

                    printf("\t\t\tCarrier 0x%02x:", pcarr_stats_entry->carrier_ref);
                    for (int j = 0; j < 29; j++) {
                        k += pcarr_stats_entry->stats[j];
                    }

                    if (k == 0) {
                        printf("\tNo calls.\n");
                    } else {
                        for (int j = 0; j < 29; j++) {
                            if (j % 2 == 0) printf(" |\n\t\t\t\t");
                            printf("| stats[%15s] =%5d\t\t", stats_to_str_lut[j], pcarr_stats_entry->stats[j]);
                        }
                        printf("\n");
                    }
                }

                dont_send_reply = 1;
                break;
            }
            case DLOG_MT_CARRIER_STATS_EXP: {

                dlog_mt_carrier_stats_exp_t *carr_stats = (dlog_mt_carrier_stats_exp_t *)ppayload;
                ppayload += sizeof(dlog_mt_carrier_stats_exp_t);
                printf("\t\tExpanded Carrier Statistics: From: %s, to: %s:\n",
                        timestamp_to_string(carr_stats->timestamp, timestamp_str, sizeof(timestamp_str)),
                        timestamp_to_string(carr_stats->timestamp2, timestamp2_str, sizeof(timestamp2_str)));

                for (int carrier = 0; carrier < CARRIER_STATS_EXP_MAX_CARRIERS; carrier++) {
                    carrier_stats_exp_entry_t *pcarr_stats_entry = &carr_stats->carrier[carrier];

                    printf("\t\t\tCarrier Ref: %d (0x%02x): ", pcarr_stats_entry->carrier_ref, pcarr_stats_entry->carrier_ref);

                    /* If no calls have been made using this carrier, skip it. */
                    if (pcarr_stats_entry->total_call_duration == 0) {
                            printf("No calls.\n");
                            continue;
                        }

                    printf("Stats vintage: %d\n", carr_stats->stats_vintage);

                    for (int j = 0; j < STATS_EXP_CALL_TYPE_MAX; j++) {
                        printf("\t\t\t\t%s stats:\t", stats_call_type_str_lut[j]);
                        for (int i = 0; i < STATS_EXP_PAYMENT_TYPE_MAX; i++) {
                            printf("%d, ", pcarr_stats_entry->stats[j][i]);
                        }
                        printf("\n");
                    }

                    printf("\t\t\t\tOperator Assisted Call Count: %d\n", pcarr_stats_entry->operator_assist_call_count);
                    printf("\t\t\t\t0+ Call Count: %d\n", pcarr_stats_entry->zero_plus_call_count);
                    printf("\t\t\t\tFree Feature B Call Count: %d\n", pcarr_stats_entry->free_featb_call_count);
                    printf("\t\t\t\tDirectory Assistance Call Count: %d\n", pcarr_stats_entry->directory_assist_call_count);
                    printf("\t\t\t\tTotal Call duration: %u\n", pcarr_stats_entry->total_call_duration);
                    printf("\t\t\t\tTotal Insert Mode Calls: %d\n", pcarr_stats_entry->total_insert_mode_calls);
                    printf("\t\t\t\tTotal Manual Mode Calls: %d\n", pcarr_stats_entry->total_manual_mode_calls);
                }

                dont_send_reply = 1;

                break;
            }
            case DLOG_MT_SUMMARY_CALL_STATS: {
                dlog_mt_summary_call_stats_t *dlog_mt_summary_call_stats = (dlog_mt_summary_call_stats_t *)ppayload;
                ppayload += sizeof(dlog_mt_summary_call_stats_t);

                printf("\t\t\tSummary Call Statistics: From: %s, to: %s:\n",
                        timestamp_to_string(dlog_mt_summary_call_stats->timestamp, timestamp_str, sizeof(timestamp_str)),
                        timestamp_to_string(dlog_mt_summary_call_stats->timestamp2, timestamp2_str, sizeof(timestamp2_str)));

                for (int j = 0; j < 16; j++) {
                    printf("\t\t\t\t%s: %5d\n", TCALSTE_stats_to_str_lut[j], dlog_mt_summary_call_stats->stats[j]);
                }
                printf("\n\t\t\t\tRep Dialer Peg Counts:\t");

                for (int j = 0; j < 10; j++) {
                    if (j == 5) {
                        printf("\n\t\t\t\t\t\t\t");
                    }
                    printf("%d, ", dlog_mt_summary_call_stats->rep_dialer_peg_count[j]);
                }

                printf("\n\t\t\t\tTotal Call duration: %us\n",
                    dlog_mt_summary_call_stats->total_call_duration);

                printf("\t\t\t\tTotal Off-hook duration: %us\n",
                    dlog_mt_summary_call_stats->total_time_off_hook);

                printf("\t\t\t\tFree Feature B Call Count: %d\n", dlog_mt_summary_call_stats->free_featb_call_count);
                printf("\t\t\t\tCompleted 1-800 billable Count: %d\n", dlog_mt_summary_call_stats->completed_1800_billable_count);
                printf("\t\t\t\tDatajack calls attempted: %d\n", dlog_mt_summary_call_stats->datajack_calls_attempt_count);
                printf("\t\t\t\tDatajack calls completed: %d\n", dlog_mt_summary_call_stats->datajack_calls_complete_count);

                dont_send_reply = 1;
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
                dont_send_reply = 0;
                context->trans_data_in_progress = 0;
                if (context->cdr_ack_buffer_len == 0) {
                    if (pack_payload - ack_payload == 0) {
                        printf("Sending empty DLOG_MT_END_DATA.\n");
                        *pack_payload++ = DLOG_MT_END_DATA;
                        end_of_data = 0;
                    } else {
                        printf("Appending DLOG_MT_END_DATA to reply.\n");
                        *pack_payload++ = DLOG_MT_END_DATA;
                        end_of_data = 0;
                    }
                } else {
                    memcpy(pack_payload, context->cdr_ack_buffer, context->cdr_ack_buffer_len);
                    pack_payload += context->cdr_ack_buffer_len;
                    printf("Sending CDR ACKs in response to DLOG_MT_END_DATA.\n");
                    end_of_data = 1;
                    context->cdr_ack_buffer_len = 0;
                }

                break;
            case DLOG_MT_TAB_UPD_ACK:
                printf("\tDLOG_MT_TAB_UPD_ACK for table 0x%02x.\n", *ppayload);
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
        send_mm_table(context, ack_payload, (int)(pack_payload - ack_payload));

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
                generate_call_in_parameters(context, &table_buffer, &table_len);
                break;
            case DLOG_MT_NCC_TERM_PARAMS:
                generate_term_access_parameters(context, &table_buffer, &table_len);
                break;
            case DLOG_MT_CALL_STAT_PARMS:
                generate_call_stat_parameters(context, &table_buffer, &table_len);
                break;
            case DLOG_MT_COMM_STAT_PARMS:
                generate_comm_stat_parameters(context, &table_buffer, &table_len);
                break;
            case DLOG_MT_USER_IF_PARMS:
                generate_user_if_parameters(context, &table_buffer, &table_len);
                break;
            case DLOG_MT_END_DATA:
                generate_dlog_mt_end_data(context, &table_buffer, &table_len);
                break;
            default:
                printf("\t");
                status = load_mm_table(context, table_list[table_index], &table_buffer, &table_len);

                /* If table can't be loaded, continue to the next. */
                if (status != 0) {
                    if (table_buffer != NULL) free(table_buffer);
                    table_buffer = NULL;
                    continue;
                }
                break;
        }
        send_mm_table(context, table_buffer, table_len);

        /* For all tables except END_OF_DATA, expect a table ACK. */
        if (table_list[table_index] != DLOG_MT_END_DATA) {
            wait_for_table_ack(context, table_buffer[0]);
        }
        free(table_buffer);
        table_buffer = NULL;
    }

    return 0;
}


int send_mm_table(mm_context_t *context, uint8_t *payload, int len)
{
    int bytes_remaining;
    int chunk_len;
    uint8_t *p = payload;
    uint8_t table_id;

    table_id = payload[0];
    bytes_remaining = len;
    printf("\tSending Table ID %d (0x%02x) %s...\n", table_id, table_id, table_to_string(table_id));
    while (bytes_remaining > 0) {
        if (bytes_remaining > PKT_TABLE_DATA_LEN_MAX) {
            chunk_len = PKT_TABLE_DATA_LEN_MAX;
        } else {
            chunk_len = bytes_remaining;
        }

        send_mm_packet(context, p, chunk_len, 0);

        if (wait_for_mm_ack(context) != 0) return -1;
        p += chunk_len;
        bytes_remaining -= chunk_len;
        printf("\tTable %d (0x%02x) %s progress: (%3d%%) - %4d / %4d\n",
            table_id, table_id, table_to_string(table_id),
            (int)((p - payload) * 100) / len, (int)(p - payload), len);
    }

    return 0;
}


int wait_for_table_ack(mm_context_t *context, uint8_t table_id)
{
    mm_packet_t packet = { 0 };
    mm_packet_t *pkt = &packet;
    int status;

    if (context->debuglevel > 1) printf("Waiting for ACK for table %d (0x%02x)\n", table_id, table_id);

    if ((status = receive_mm_packet(context, pkt)) == PKT_SUCCESS) {
        context->rx_seq = pkt->hdr.flags & FLAG_SEQUENCE;

        if (pkt->payload_len >= PKT_TABLE_ID_OFFSET) {
            phone_num_to_string(context->terminal_id, sizeof(context->terminal_id), pkt->payload, PKT_TABLE_ID_OFFSET);
            if (context->debuglevel > 1) printf("Received packet from phone# %s\n", context->terminal_id);
            if (context->debuglevel > 2) print_mm_packet(RX, pkt);

            if((pkt->payload[PKT_TABLE_ID_OFFSET] == DLOG_MT_TAB_UPD_ACK) &&
               (pkt->payload[PKT_TABLE_DATA_OFFSET] == table_id)) {
                if (context->debuglevel > 0) printf("Seq: %d: Received ACK for table %d (0x%02x)\n", context->rx_seq, table_id, table_id);
                send_mm_ack(context, 0);
            } else {
                printf("%s: Error: Received ACK for wrong table, expected %d (0x%02x), received %d (0x%02x)\n",
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
        fclose(stream);
        return -1;
    }

    bufp = *buffer;
    *bufp++ = table_id;

    *len = 1;

    while (1) {
        uint8_t c = (uint8_t)fgetc(stream);
        if(feof(stream)) {
            break;
        }
        *bufp++ = c;
        *len = *len + 1;
        if (*len > size) {
            fclose(stream);
            return -1;
        }
    }

    printf("Loaded table ID %d (0x%02x) from %s (%d bytes).\n", table_id, table_id, fname, *len - 1);
    fclose(stream);

    return 0;
}

int rewrite_instserv_parameters(char *access_code, dlog_mt_install_params_t *pinstsv_table, char *filename)
{
    FILE *ostream = NULL;

    int i;
    if (strnlen(access_code, 8) != ACCESS_CODE_LEN) {
        printf("Error: Access Code must be 7-digits\n");
        return (-1);
    }

    // Rewrite table with our Access Code
    for (i = 0; i < ACCESS_CODE_LEN; i++) {
        if (i % 2 == 0) {
            pinstsv_table->access_code[i >> 1]  = (access_code[i] - '0') << 4;
        } else {
            pinstsv_table->access_code[i >> 1] |= (access_code[i] - '0');
        }
    }
    pinstsv_table->access_code[3] |= 0x0e;   /* Terminate the Access Code with 0xe */

    if(!(ostream = fopen(filename, "wb"))) {
        printf("Error opening %s\n", filename);
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

void generate_term_access_parameters(mm_context_t *context, uint8_t **buffer, int *len)
{
    int i;
    dlog_mt_ncc_term_params_t *pncc_term_params;
    uint8_t *pbuffer;

    *len = sizeof(dlog_mt_ncc_term_params_t) + 1;
    pbuffer = calloc(1, *len);
    if (pbuffer == NULL) return;
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
    if (strnlen(context->ncc_number[1], sizeof(context->ncc_number[1])) > 0) {
        printf("\tSecondary NCC: %s\n", context->ncc_number[1]);
        string_to_bcd_a(context->ncc_number[1], pncc_term_params->sec_ncc_number, sizeof(pncc_term_params->sec_ncc_number));
    }

    *buffer = pbuffer;

}

void generate_call_in_parameters(mm_context_t* context, uint8_t** buffer, int* len)
{
    dlog_mt_call_in_params_t* pcall_in_params;
    uint8_t* pbuffer;
    time_t rawtime;
    struct tm* ptm;
    uint8_t call_in_hour;

    *len = sizeof(dlog_mt_call_in_params_t) + 1;
    pbuffer = calloc(1, *len);
    if (pbuffer == NULL) return;
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
}

void generate_call_stat_parameters(mm_context_t *context, uint8_t **buffer, int *len)
{
    int i;
    dlog_mt_call_stat_params_t *pcall_stat_params;
    uint8_t *pbuffer;

    *len = sizeof(dlog_mt_call_stat_params_t) + 1;
    pbuffer = calloc(1, *len);
    if (pbuffer == NULL) return;
    pbuffer[0] = DLOG_MT_CALL_STAT_PARMS;

    pcall_stat_params = (dlog_mt_call_stat_params_t *)&pbuffer[1];

    printf("\nGenerating Call Stat Parameters table:\n");
    pcall_stat_params->callstats_start_time[0] = 0; /* HH */
    pcall_stat_params->callstats_start_time[1] = 0; /* MM */
    pcall_stat_params->callstats_duration = 1;      /* Indicates the number of days over which call statistics will be accumulated. */
    pcall_stat_params->callstats_threshold = 10;
    pcall_stat_params->timestamp[0][0] = 0;         /* HH */
    pcall_stat_params->timestamp[0][1] = 0;         /* MM */
    pcall_stat_params->timestamp[1][0] = 0;         /* HH */
    pcall_stat_params->timestamp[1][1] = 0;         /* MM */
    pcall_stat_params->timestamp[2][0] = 0;         /* HH */
    pcall_stat_params->timestamp[2][1] = 0;         /* MM */
    pcall_stat_params->timestamp[3][0] = 0;         /* HH */
    pcall_stat_params->timestamp[3][1] = 0;         /* MM */
    pcall_stat_params->enable = 4;
    pcall_stat_params->cdr_threshold = 50;
    pcall_stat_params->cdr_start_time[0] = 0;       /* HH */
    pcall_stat_params->cdr_start_time[1] = 0;       /* MM */
    pcall_stat_params->cdr_duration_days = 255;
    pcall_stat_params->cdr_duration_hours_flags = 0;
    *buffer = pbuffer;

    printf("\tStart time:     %02d:%02d\n",
        pcall_stat_params->callstats_start_time[0], pcall_stat_params->callstats_start_time[1]);
    printf("\tDuration:       %02dd\n", pcall_stat_params->callstats_duration);
    printf("\tThreshold:      %02d\n", pcall_stat_params->callstats_threshold);

    for(i = 0; i < 4; i++) {
        printf("\tTimestamp[%d]:   %02d:%02d\n", i,
            pcall_stat_params->timestamp[i][0], pcall_stat_params->timestamp[i][1]);
    }

    printf("\tEnable:         0x%02x\n", pcall_stat_params->enable);
    printf("\tCDR Threshold:  %d\n", pcall_stat_params->cdr_threshold);
    printf("\tCDR Start Time: %02d:%02d\n",
        pcall_stat_params->cdr_start_time[0], pcall_stat_params->cdr_start_time[1]);
    printf("\tCDR Duration:   %dd:%dh\n", pcall_stat_params->cdr_duration_days, pcall_stat_params->cdr_duration_hours_flags & 0x1f);
    printf("\tCDR Flags:      0x%02x\n", pcall_stat_params->cdr_duration_hours_flags & 0xe0);
}

void generate_comm_stat_parameters(mm_context_t *context, uint8_t **buffer, int *len)
{
    int i;
    dlog_mt_comm_stat_params_t *pcomm_stat_params;
    uint8_t *pbuffer;

    *len = sizeof(dlog_mt_comm_stat_params_t) + 1;
    pbuffer = calloc(1, *len);
    if (pbuffer == NULL) return;
    pbuffer[0] = DLOG_MT_COMM_STAT_PARMS;

    pcomm_stat_params = (dlog_mt_comm_stat_params_t *)&pbuffer[1];

    printf("\nGenerating Comm Stat Parameters table:\n");
    pcomm_stat_params->co_access_dial_complete = 100;
    pcomm_stat_params->co_access_dial_complete_int = 100;
    pcomm_stat_params->dial_complete_carr_detect = 100;
    pcomm_stat_params->dial_complete_carr_detect_int = 100;
    pcomm_stat_params->carr_detect_first_pac = 100;
    pcomm_stat_params->carr_detect_first_pac_int = 100;
    pcomm_stat_params->user_waiting_expect_info = 600;
    pcomm_stat_params->user_waiting_expect_info_int = 100;
    pcomm_stat_params->perfstats_threshold = 1;
    pcomm_stat_params->perfstats_start_time[0] = 0;         /* HH */
    pcomm_stat_params->perfstats_start_time[1] = 0;         /* MM */
    pcomm_stat_params->perfstats_duration = 1;              /* Indicates the number of days over which perf statistics will be accumulated. */
    pcomm_stat_params->perfstats_timestamp[0][0] = 0;       /* HH */
    pcomm_stat_params->perfstats_timestamp[0][1] = 0;       /* MM */
    pcomm_stat_params->perfstats_timestamp[1][0] = 0;       /* HH */
    pcomm_stat_params->perfstats_timestamp[1][1] = 0;       /* MM */
    pcomm_stat_params->perfstats_timestamp[2][0] = 0;       /* HH */
    pcomm_stat_params->perfstats_timestamp[2][1] = 0;       /* MM */
    pcomm_stat_params->perfstats_timestamp[3][0] = 0;       /* HH */
    pcomm_stat_params->perfstats_timestamp[3][1] = 0;       /* MM */
    *buffer = pbuffer;

    printf("\tStart time:     %02d:%02d\n",
        pcomm_stat_params->perfstats_start_time[0], pcomm_stat_params->perfstats_start_time[1]);
    printf("\tDuration:       %02dd\n", pcomm_stat_params->perfstats_duration);
    printf("\tThreshold:      %02d\n", pcomm_stat_params->perfstats_threshold);

    for(i = 0; i < 4; i++) {
        printf("\tTimestamp[%d]:   %02d:%02d\n", i,
            pcomm_stat_params->perfstats_timestamp[i][0], pcomm_stat_params->perfstats_timestamp[i][1]);
    }
}

void generate_user_if_parameters(mm_context_t *context, uint8_t **buffer, int *len)
{
    dlog_mt_user_if_params_t *puser_if_params;
    uint8_t *pbuffer;

    *len = sizeof(dlog_mt_user_if_params_t) + 1;
    pbuffer = calloc(1, *len);
    if (pbuffer == NULL) return;
    pbuffer[0] = DLOG_MT_USER_IF_PARMS;

    puser_if_params = (dlog_mt_user_if_params_t *)&pbuffer[1];

    printf("\nGenerating User Interface Parameters table:\n");
    puser_if_params->digit_clear_delay = 450;
    puser_if_params->transient_delay = 450;
    puser_if_params->transient_hint_time = 450;
    puser_if_params->visual_to_voice_delay = 450;
    puser_if_params->voice_repitition_delay = 450;
    puser_if_params->no_action_timeout = 3000;
    puser_if_params->card_validation_timeout = 4500;    /* Change from 30s to 45s */
    puser_if_params->dj_second_string_dtmf_timeout = 300;
    puser_if_params->spare_timer_b = 1100;
    puser_if_params->cp_input_timeout = 100;
    puser_if_params->language_timeout = 1000;
    puser_if_params->cfs_timeout = 200;
    puser_if_params->called_party_disconnect = 1200;
    puser_if_params->no_voice_prompt_reps = 3;
    puser_if_params->accs_digit_timeout = 450;
    puser_if_params->collect_call_timeout = 400;
    puser_if_params->bong_tone_timeout = 300;
    puser_if_params->accs_no_action_timeout = 450;
    puser_if_params->card_auth_required_timeout = 4000;
    puser_if_params->rate_request_timeout = 6000;
    puser_if_params->manual_dial_hold_time = 50;
    puser_if_params->autodialer_hold_time = 300;
    puser_if_params->coin_first_warning_time = 30;
    puser_if_params->coin_second_warning_time = 5;
    puser_if_params->alternate_bong_tone_timeout = 200;
    puser_if_params->delay_after_bong_tone = 125;
    puser_if_params->alternate_delay_after_bong_tone = 125;
    puser_if_params->display_scroll_speed = 0;
    puser_if_params->aos_bong_tone_timeout = 600;
    puser_if_params->fgb_aos_second_spill_timeout = 600;
    puser_if_params->datajack_connect_timeout = 15000;
    puser_if_params->datajack_pause_threshold = 45;
    puser_if_params->datajack_ias_timer = 10;
    *buffer = pbuffer;

    printf("\tDigit clear delay:           %5d\n", puser_if_params->digit_clear_delay);
    printf("\tTransient Delay:             %5d\n", puser_if_params->transient_delay);
    printf("\tTransient Hint Time:         %5d\n", puser_if_params->transient_hint_time);
    printf("\tVisual to Voice Delay:       %5d\n", puser_if_params->visual_to_voice_delay);
    printf("\tVoice Repetition Delay:      %5d\n", puser_if_params->voice_repitition_delay);
    printf("\tNo Action Timeout:           %5d\n", puser_if_params->no_action_timeout);
    printf("\tCard Validation Timeout:     %5d\n", puser_if_params->card_validation_timeout);
    printf("\tDJ 2nd String DTMF Timeout:  %5d\n", puser_if_params->dj_second_string_dtmf_timeout);
    printf("\tCP Input Timeout:            %5d\n", puser_if_params->cp_input_timeout);
    printf("\tLanguage Timeout:            %5d\n", puser_if_params->language_timeout);
    printf("\tCFS Timeout:                 %5d\n", puser_if_params->cfs_timeout);
    printf("\tCPD Timeout:                 %5d\n", puser_if_params->called_party_disconnect);
    printf("\t# Voice Prompt Repititions:     %2d\n", puser_if_params->no_voice_prompt_reps);
    printf("\tACCS Digit Timeout:          %5d\n", puser_if_params->accs_digit_timeout);
    printf("\tCollect Call Timeout:        %5d\n", puser_if_params->collect_call_timeout);
    printf("\tBong Tone Timeout:           %5d\n", puser_if_params->bong_tone_timeout);
    printf("\tACCS No Action Timeout:      %5d\n", puser_if_params->accs_no_action_timeout);
    printf("\tCCard Auth Required Timeout: %5d\n", puser_if_params->card_auth_required_timeout);
    printf("\tRate Request Timeout:        %5d\n", puser_if_params->rate_request_timeout);
    printf("\tManual Dial Hold Time:       %5d\n", puser_if_params->manual_dial_hold_time);
    printf("\tAutodialer Hold Time:        %5d\n", puser_if_params->autodialer_hold_time);
    printf("\tCoin First Warning Time:     %5d\n", puser_if_params->coin_first_warning_time);
    printf("\tCoin Second Warning Time:    %5d\n", puser_if_params->coin_second_warning_time);
    printf("\tAlt Bong Tone Timeout:       %5d\n", puser_if_params->alternate_bong_tone_timeout);
    printf("\tDelay After Bong Tone:       %5d\n", puser_if_params->delay_after_bong_tone);
    printf("\tAlt Delay After Bong Tone:   %5d\n", puser_if_params->alternate_delay_after_bong_tone);
    printf("\tDisplay Scroll Speed:        %5d\n", puser_if_params->display_scroll_speed);
    printf("\tAOS Bong Tone Timeout:       %5d\n", puser_if_params->aos_bong_tone_timeout);
    printf("\tFGB AOS Second Spill Timeout:%5d\n", puser_if_params->fgb_aos_second_spill_timeout);
    printf("\tDatajack Connect Timeout:    %5d\n", puser_if_params->datajack_connect_timeout);
    printf("\tDatajack Pause Threshold:    %5d\n", puser_if_params->datajack_pause_threshold);
    printf("\tDatajack IAS Timer:          %5d\n", puser_if_params->datajack_ias_timer);
}

void generate_dlog_mt_end_data(mm_context_t *context, uint8_t **buffer, int *len)
{
    *len = 1;
    *buffer = calloc(1, *len);
    if (*buffer == NULL) return;
    *buffer[0] = DLOG_MT_END_DATA;
}

int create_terminal_specific_directory(char *table_dir, char *terminal_id) {
    char dirname[80];
    int status = 0;
    errno = 0;

    snprintf(dirname, sizeof(dirname), "%s/%s", table_dir, terminal_id);

#ifdef _WIN32
    status = _mkdir(dirname);
#else
    status = mkdir(dirname, 0755);
#endif
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
