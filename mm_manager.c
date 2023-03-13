/*
 * This is a "Manager" for the Nortel Millennium payhone.
 *
 * It can provision a Nortel Millennium payphone with Rev 1.0 or 1.3
 * Control PCP.  CDRs, Alarms, and Maintenance Reports can also be
 * retieved.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2023, Howard M. Harte
 */

#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>  /* String function definitions */
#include <inttypes.h>
#ifndef _WIN32
# include <unistd.h> /* UNIX standard function definitions */
# include <libgen.h>
# include <signal.h>
#else  /* ifndef _WIN32 */
# include <direct.h>
# include "third-party/getopt.h"
# include <windows.h>
#endif /* ifndef _WIN32 */
#include <errno.h> /* Error number definitions */
#include <time.h>  /* time_t, struct tm, time, gmtime */

#include <sys/stat.h>

#include "./mm_manager.h"
#include "./mm_serial.h"
#include "./mm_udp.h"

#ifndef VERSION
# define VERSION "Unknown"
#endif /* VERSION */

#define JAN12020 1577865600

/* Function Prototypes */
time_t mm_time(int test_mode, time_t* rawtime);

static int mm_shutdown(mm_context_t* context);
static int mm_download_tables(mm_context_t* context, char* terminal_id);
static int load_mm_table(mm_context_t* context, char* terminal_id, uint8_t table_id, uint8_t** buffer, size_t* len);
static void generate_install_parameters(mm_context_t* context, uint8_t** buffer, size_t* len);
static void generate_term_access_parameters(mm_context_t* context, char* terminal_id, uint8_t** buffer, size_t* len);
static void generate_term_access_parameters_mtr1(mm_context_t* context, char* terminal_id, uint8_t** buffer, size_t* len);
static void generate_call_in_parameters(mm_context_t* context, uint8_t** buffer, size_t* len);
static void generate_call_stat_parameters(mm_context_t* context, uint8_t** buffer, size_t* len);
static void generate_comm_stat_parameters(mm_context_t* context, uint8_t** buffer, size_t* len);
static void generate_user_if_parameters(mm_context_t* context, uint8_t** buffer, size_t* len);
static void generate_dlog_mt_end_data(mm_context_t* context, uint8_t** buffer, size_t* len);
static int process_mm_table(mm_context_t* context, mm_table_t* table);
static int create_terminal_specific_directory(char* table_dir, char* terminal_id);
static int update_terminal_download_time(mm_context_t* context, char* terminal_id);
static int check_mm_table_is_newer(mm_context_t* context, char* terminal_id, uint8_t table_id);
static void mm_display_help(const char* name, FILE* stream);
#ifndef _WIN32
void signal_handler(int sig);
#endif

extern const char* modem_responses[];

/* Terminal Table Lists for various MTR versions. */
uint8_t table_list_mtr_2x[] = {
    DLOG_MT_NCC_TERM_PARAMS,    /* Required */
    DLOG_MT_FCONFIG_OPTS,       /* Required */
    DLOG_MT_ADVERT_PROMPTS,
    DLOG_MT_USER_IF_PARMS,
    DLOG_MT_INSTALL_PARAMS,     /* Required */
    DLOG_MT_COMM_STAT_PARMS,
    DLOG_MT_MODEM_PARMS,
    DLOG_MT_CALL_STAT_PARMS,
    DLOG_MT_CALL_IN_PARMS,
    DLOG_MT_COIN_VAL_TABLE,     /* Required */
    DLOG_MT_REP_DIAL_LIST,
    DLOG_MT_LIMSERV_DATA,
    DLOG_MT_NUM_PLAN_TABLE,     /* Required */
    DLOG_MT_SPARE_TABLE,        /* 1.3 only */
    DLOG_MT_RATE_TABLE,         /* Required */
    DLOG_MT_EXP_VIS_PROPTS_L1,  /* 1.3 only */
    DLOG_MT_EXP_VIS_PROPTS_L2,  /* 1.3 only */
    DLOG_MT_CALL_SCREEN_LIST,   /* Required */
    DLOG_MT_SCARD_PARM_TABLE,   /* Required */
    DLOG_MT_CARD_TABLE_EXP,     /* Required */
    DLOG_MT_CARRIER_TABLE_EXP,  /* Required */
    DLOG_MT_NPA_NXX_TABLE_1,
    DLOG_MT_NPA_NXX_TABLE_2,
    DLOG_MT_NPA_NXX_TABLE_3,
    DLOG_MT_NPA_NXX_TABLE_4,
    DLOG_MT_NPA_NXX_TABLE_5,
    DLOG_MT_NPA_NXX_TABLE_6,
    DLOG_MT_NPA_NXX_TABLE_7,
    DLOG_MT_NPA_NXX_TABLE_8,
    DLOG_MT_NPA_NXX_TABLE_9,
    DLOG_MT_NPA_NXX_TABLE_10,
    DLOG_MT_NPA_NXX_TABLE_11,
    DLOG_MT_NPA_NXX_TABLE_12,
    DLOG_MT_NPA_NXX_TABLE_13,
    DLOG_MT_NPA_NXX_TABLE_14,
    DLOG_MT_NPA_SBR_TABLE,
    DLOG_MT_INTL_SBR_TABLE,
    DLOG_MT_NPA_NXX_TABLE_15,
    DLOG_MT_NPA_NXX_TABLE_16,
    DLOG_MT_END_DATA,
    0                        /* End of table list */
};

uint8_t table_list_mtr_120[] = {
    DLOG_MT_NCC_TERM_PARAMS,    /* Required */
    DLOG_MT_FCONFIG_OPTS,       /* Required */
    DLOG_MT_VIS_PROMPTS_L1,
    DLOG_MT_VIS_PROMPTS_L2,
    DLOG_MT_ADVERT_PROMPTS,
    DLOG_MT_USER_IF_PARMS,
    DLOG_MT_INSTALL_PARAMS,     /* Required */
    DLOG_MT_COMM_STAT_PARMS,
    DLOG_MT_MODEM_PARMS,
    DLOG_MT_CALL_STAT_PARMS,
    DLOG_MT_CALL_IN_PARMS,
    DLOG_MT_COIN_VAL_TABLE,     /* Required */
    DLOG_MT_REP_DIAL_LIST,
    DLOG_MT_LIMSERV_DATA,
    DLOG_MT_NUM_PLAN_TABLE,     /* Required */
    DLOG_MT_RATE_TABLE,         /* Required */
    DLOG_MT_CALL_SCREEN_LIST,   /* Required */
    DLOG_MT_SCARD_PARM_TABLE,   /* Required */
    DLOG_MT_CARD_TABLE_EXP,     /* Required */
    DLOG_MT_CARRIER_TABLE_EXP,  /* Required */
    DLOG_MT_NPA_NXX_TABLE_1,
    DLOG_MT_NPA_NXX_TABLE_2,
    DLOG_MT_NPA_NXX_TABLE_3,
    DLOG_MT_NPA_NXX_TABLE_4,
    DLOG_MT_NPA_NXX_TABLE_5,
    DLOG_MT_NPA_NXX_TABLE_6,
    DLOG_MT_NPA_NXX_TABLE_7,
    DLOG_MT_NPA_NXX_TABLE_8,
    DLOG_MT_NPA_NXX_TABLE_9,
    DLOG_MT_NPA_NXX_TABLE_10,
    DLOG_MT_NPA_NXX_TABLE_11,
    DLOG_MT_NPA_NXX_TABLE_12,
    DLOG_MT_NPA_NXX_TABLE_13,
    DLOG_MT_NPA_NXX_TABLE_14,
    DLOG_MT_NPA_SBR_TABLE,
    DLOG_MT_INTL_SBR_TABLE,
    DLOG_MT_NPA_NXX_TABLE_15,
    DLOG_MT_NPA_NXX_TABLE_16,
    DLOG_MT_END_DATA,
    0                          /* End of table list */
};

uint8_t table_list_mtr19[] = {
    DLOG_MT_NCC_TERM_PARAMS,    /* Required */
    DLOG_MT_CARD_TABLE,         /* MTR 1.7, 1.9 Length: 661 */
    DLOG_MT_CARRIER_TABLE,      /* MTR 1.7, 1.9 Length: 678 */
    DLOG_MT_FCONFIG_OPTS,       /* Required */
    DLOG_MT_VIS_PROMPTS_L1,
    DLOG_MT_VIS_PROMPTS_L2,
    DLOG_MT_ADVERT_PROMPTS,
    DLOG_MT_USER_IF_PARMS,
    DLOG_MT_INSTALL_PARAMS,     /* Required */
    DLOG_MT_COMM_STAT_PARMS,
    DLOG_MT_MODEM_PARMS,
    DLOG_MT_CALL_STAT_PARMS,
    DLOG_MT_CALL_IN_PARMS,
    DLOG_MT_COIN_VAL_TABLE,     /* Required */
    DLOG_MT_REP_DIAL_LIST,
    DLOG_MT_LIMSERV_DATA,
    DLOG_MT_NUM_PLAN_TABLE,     /* Required */
    DLOG_MT_RATE_TABLE,         /* Required */
    DLOG_MT_CALL_SCREEN_LIST,   /* Required, Length: 3401 */
    DLOG_MT_SCARD_PARM_TABLE,
    DLOG_MT_COMP_LCD_TABLE_1,
    DLOG_MT_COMP_LCD_TABLE_2,
    DLOG_MT_COMP_LCD_TABLE_3,
    DLOG_MT_COMP_LCD_TABLE_4,
    DLOG_MT_COMP_LCD_TABLE_5,
    DLOG_MT_COMP_LCD_TABLE_6,
    DLOG_MT_COMP_LCD_TABLE_7,
    DLOG_MT_END_DATA,
    0                         /* End of table list */
};

uint8_t table_list_mtr17[] = {
    DLOG_MT_NCC_TERM_PARAMS,    /* Required */
    DLOG_MT_CARD_TABLE,         /* MTR 1.7, 1.9 Length: 661 */
    DLOG_MT_CARRIER_TABLE,      /* MTR 1.7, 1.9 Length: 678 */
    DLOG_MT_CALLSCRN_UNIVERSAL, /* MTR 1.7 Length: 721 */
    DLOG_MT_FCONFIG_OPTS,       /* Required */
    DLOG_MT_VIS_PROMPTS_L1,
    DLOG_MT_VIS_PROMPTS_L2,
    DLOG_MT_ADVERT_PROMPTS,
    DLOG_MT_USER_IF_PARMS,
    DLOG_MT_INSTALL_PARAMS,     /* Required */
    DLOG_MT_COMM_STAT_PARMS,
    DLOG_MT_MODEM_PARMS,
    DLOG_MT_CALL_STAT_PARMS,
    DLOG_MT_CALL_IN_PARMS,
    DLOG_MT_COIN_VAL_TABLE,     /* Required */
    DLOG_MT_REP_DIAL_LIST,
    DLOG_MT_LIMSERV_DATA,
    DLOG_MT_NUM_PLAN_TABLE,     /* Required */
    DLOG_MT_RATE_TABLE,         /* Required */
    DLOG_MT_LCD_TABLE_1,        /* MTR 1.7 Length: 819 */
    DLOG_MT_LCD_TABLE_2,
    DLOG_MT_LCD_TABLE_3,
    DLOG_MT_LCD_TABLE_4,
    DLOG_MT_LCD_TABLE_5,
    DLOG_MT_LCD_TABLE_6,
    DLOG_MT_LCD_TABLE_7,
    DLOG_MT_LCD_TABLE_8,
    DLOG_MT_LCD_TABLE_9,
    DLOG_MT_LCD_TABLE_10,
    DLOG_MT_END_DATA,
    0                         /* End of table list */
};

uint8_t table_list_mtr17_intl[] = {
    DLOG_MT_NCC_TERM_PARAMS,    /* Required */
    DLOG_MT_CARD_TABLE,         /* MTR 1.7, 1.9 Length: 661 */
    DLOG_MT_CARRIER_TABLE,      /* MTR 1.7, 1.9 Length: 678 */
    DLOG_MT_FCONFIG_OPTS,       /* Required */
    DLOG_MT_VIS_PROMPTS_L1,
    DLOG_MT_VIS_PROMPTS_L2,
    DLOG_MT_ADVERT_PROMPTS,
    DLOG_MT_USER_IF_PARMS,
    DLOG_MT_INSTALL_PARAMS,     /* Required */
    DLOG_MT_COMM_STAT_PARMS,
    DLOG_MT_MODEM_PARMS,
    DLOG_MT_CALL_STAT_PARMS,
    DLOG_MT_CALL_IN_PARMS,
    DLOG_MT_COIN_VAL_TABLE,     /* Required */
    DLOG_MT_REP_DIAL_LIST,
    DLOG_MT_LIMSERV_DATA,
    DLOG_MT_CALLSCRN_EXP,
    DLOG_MT_NUM_PLAN_TABLE,     /* Required */
    DLOG_MT_RATE_TABLE,         /* Required */
    DLOG_MT_LCD_TABLE_1,        /* MTR 1.7 Length: 819 */
    DLOG_MT_LCD_TABLE_2,
    DLOG_MT_LCD_TABLE_3,
    DLOG_MT_LCD_TABLE_4,
    DLOG_MT_LCD_TABLE_5,
    DLOG_MT_LCD_TABLE_6,
    DLOG_MT_LCD_TABLE_7,
    DLOG_MT_END_DATA,
    0                         /* End of table list */
};

const char cmdline_options[] = "a:b:cd:e:f:hi:k:l:mn:p:qrst:uvw";
#define DEFAULT_MODEM_RESET_STRING "ATZ"
#define DEFAULT_MODEM_INIT_STRING "ATE=1 S0=1 S7=3 &D2 +MS=B212"

volatile int inject_comm_error = 0;

#ifdef _WIN32
int manager_running = 1;

BOOL WINAPI signal_handler(DWORD dwCtrlType) {
    switch (dwCtrlType)
    {
    case CTRL_C_EVENT:
        printf("\nReceived ^C, wait for shutdown.\n");
        manager_running = 0;
        return TRUE;
    case CTRL_BREAK_EVENT:
        printf("\nReceived ^BREAK, inject communication error.\n");
        inject_comm_error = 1;
        return TRUE;
    default:
        printf("Event: %lu\n", dwCtrlType);
        return FALSE;
    }
}
#else
volatile sig_atomic_t manager_running = 1;

void signal_handler(int sig) {
    switch (sig) {
    case SIGINT:
        printf("\nReceived ^C, wait for shutdown.\n");
        manager_running = 0;
        break;
    default:
        printf("Received signal %d\n", sig);
    }
}
#endif /* _WIN32 */

int main(int argc, char *argv[]) {
    mm_context_t *mm_context;
    mm_table_t    mm_table;
    char *modem_dev = NULL;
    int   ncc_index = 0;
    int   c;
    int   baudrate      = 19200;
    char  access_code_str[8];
    char  key_card_number_str[11];
    int   quiet = 0;
    int   status;
    int   retries;

    time_t rawtime;
    struct tm ptm = { 0 };

#ifdef _WIN32
    SetConsoleCtrlHandler(signal_handler, TRUE);
#else
    signal(SIGINT, signal_handler);
#endif /* _WIN32 */

    opterr = 0;

    if (argc < 2) {
        mm_display_help(basename(argv[0]), stderr);
        fprintf(stderr, "\nError: at least -f <filename> must be specified.\n");
        exit (-EINVAL);
    }

    mm_context = (mm_context_t *)calloc(1, sizeof(mm_context_t));

    if (mm_context == NULL) {
        printf("Error: failed to allocate %d bytes.\n", (int)sizeof(mm_context_t));
        exit (-ENOMEM);
    }

    snprintf(mm_context->default_table_dir,  sizeof(mm_context->default_table_dir),  "tables/default");
    snprintf(mm_context->term_table_dir,     sizeof(mm_context->term_table_dir),     "tables");
    snprintf(mm_context->connection.modem_reset_string, sizeof(mm_context->connection.modem_reset_string), "%s", DEFAULT_MODEM_RESET_STRING);
    snprintf(mm_context->connection.modem_init_string,  sizeof(mm_context->connection.modem_init_string), "%s",  DEFAULT_MODEM_INIT_STRING);

    mm_context->connection.proto.rx_packet_gap = 10;

    mm_context->access_code[0] = 0x27;
    mm_context->access_code[1] = 0x27;
    mm_context->access_code[2] = 0x37;
    mm_context->access_code[3] = 0x8e;

    mm_context->key_card_number[0] = 0x40;
    mm_context->key_card_number[1] = 0x12;
    mm_context->key_card_number[2] = 0x88;
    mm_context->key_card_number[3] = 0x88;
    mm_context->key_card_number[4] = 0x88;

    mm_context->complete_download = FALSE;
    mm_context->connection.proto.monitor_carrier = TRUE;

    mm_context->test_mode = TRUE;

    /* Parse command line to get -q (quiet) option. */
    while ((c = getopt(argc, argv, cmdline_options)) != -1) {
        switch (c) {
            case 'q':
                quiet = 1;
                break;
            default:
                break;
        }
    }

    if (quiet == 0) {
        printf("mm_manager v0.8 [%s] - (c) 2020-2023, Howard M. Harte\n\n", VERSION);
    }

    /* Parse command line again to get the rest of the options. */
    optind = 1;

    while ((c = getopt(argc, argv, cmdline_options)) != -1) {
        switch (c) {
            case 'a':
            {
                if (strnlen(optarg, 7) != 7) {
                    fprintf(stderr, "Option -a takes a 7-digit access code.\n");
                    mm_shutdown(mm_context);
                    return(-EINVAL);
                }

                /* Update Access Code */
                for (int i = 0; i < ACCESS_CODE_LEN; i++) {
                    if (i % 2 == 0) {
                        mm_context->access_code[i >> 1] = (optarg[i] - '0') << 4;
                    }
                    else {
                        mm_context->access_code[i >> 1] |= (optarg[i] - '0');
                    }
                }

                mm_context->access_code[3] |= 0x0e; /* Terminate the Access Code with 0xe */
                break;
            }
            case 'b':
                baudrate = atoi(optarg);
                break;
            case 'c':
                fprintf(stdout, "NOTE: Complete set of tables will be downloaded for every download request.\n");
                mm_context->complete_download = TRUE;
                break;
            case 'd':
                snprintf(mm_context->default_table_dir, sizeof(mm_context->default_table_dir), "%s", optarg);
                break;
            case 'e':
                mm_context->connection.proto.error_inject_type = atoi(optarg);
                if (mm_context->connection.proto.error_inject_type < 5) {
                    printf("SIGBRK will inject %s.\n", error_inject_type_to_str(mm_context->connection.proto.error_inject_type));
                }
                else {
                    fprintf(stderr, "Error: -e <inject_error_type> must be one of:\n");
                    for (int i = 0; i < 5; i++) {
                        fprintf(stderr, "\t%d - %s\n", i, error_inject_type_to_str(i));
                    }
                    mm_shutdown(mm_context);
                    return(-EINVAL);
                }
                break;
            case 'f':
                modem_dev = optarg;
                break;
            case 'h':
                mm_display_help(basename(argv[0]), stdout);
                mm_shutdown(mm_context);
                return(0);
                break;
            case 'i':
                snprintf(mm_context->connection.modem_init_string, sizeof(mm_context->connection.modem_init_string), "%s", optarg);
                break;
            case 'k':
            {
                if (strnlen(optarg, 10) != 10) {
                    fprintf(stderr, "Option -k takes a 10-digit key code.\n");
                    mm_shutdown(mm_context);
                    return(-EINVAL);
                }

                /* Update Key Card Number */
                for (int i = 0; i < KEY_CARD_LEN; i++) {
                    if (i % 2 == 0) {
                        mm_context->key_card_number[i >> 1] = (optarg[i] - '0') << 4;
                    }
                    else {
                        mm_context->key_card_number[i >> 1] |= (optarg[i] - '0');
                    }
                }
                break;
            }
            case 'l':
                if (!(mm_context->connection.logstream = fopen(optarg, "w"))) {
                    fprintf(stderr, "mm_manager: Can't write log file '%s': %s\n", optarg, strerror(errno));
                    mm_shutdown(mm_context);
                    return(-ENOENT);
                }
                break;
            case 'm':
                mm_context->connection.proto.use_modem = TRUE;
                mm_context->test_mode = FALSE;
                break;
            case 'n':
                if (ncc_index > 1) {
                    fprintf(stderr, "-n may only be specified twice.\n");
                    mm_shutdown(mm_context);
                    return(-EINVAL);
                }

                if ((strnlen(optarg, 16) < 1) || (strnlen(optarg, 16) > 15)) {
                    fprintf(stderr, "Option -n takes a 1- to 15-digit NCC number.\n");
                    mm_shutdown(mm_context);
                    return(-EINVAL);
                }
                else {
                    snprintf(mm_context->ncc_number[ncc_index], sizeof(mm_context->ncc_number[0]), "%s", optarg);
                    ncc_index++;
                }
                break;
            case 'p':
                if (mm_create_pcap(optarg, &mm_context->connection.proto.pcapstream) != 0) {
                    fprintf(stderr, "mm_manager: Can't write packet capture file '%s': %s\n", optarg, strerror(errno));
                    mm_shutdown(mm_context);
                    return(-EINVAL);
                }
                break;
            case 'q':
                break;
            case 'r':
                printf("NOTE: Rating test mode enabled.\n");
                mm_context->rating_test_mode = 1;
                break;
            case 's':
                printf("NOTE: Using minimum required table list for download.\n");
                mm_context->minimal_table_set = 1;
                break;
            case 't':
                snprintf(mm_context->term_table_dir,    sizeof(mm_context->term_table_dir),    "%s", optarg);
                break;
            case 'u':
                printf("Sending UDP packets to 127.0.0.1:%d\n", MM_UDP_PORT);
                if (mm_create_udp("127.0.0.1", MM_UDP_PORT) != 0) {
                    fprintf(stderr, "mm_create_udp() failed.\n");
                    mm_shutdown(mm_context);
                    return(-EINVAL);
                }
                mm_context->connection.proto.send_udp = 1;
                break;
            case 'v':
                mm_context->debuglevel++;
                mm_context->connection.proto.debuglevel++;
                break;
            case 'w':   /* Don't monitor carrier detect signal from modem. */
                mm_context->connection.proto.monitor_carrier = FALSE;
                break;
            case '?':
            default:
                if ((optopt == 'f') || (optopt == 'l') || (optopt == 'a') || (optopt == 'n') || (optopt == 'b')) {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                }
                mm_shutdown(mm_context);
                exit(-EINVAL);
                break;
            }
    }

    if (optind < argc) {
        for (c = optind; c < argc; c++) {
            fprintf(stderr, "Error: superfluous non-option argument '%s'.\n", argv[c]);
        }
        mm_shutdown(mm_context);
        return(-EINVAL);
    }

    printf("Default Table directory: %s\n",                         mm_context->default_table_dir);
    printf("Terminal-specific Table directory: %s/<terminal_id>\n", mm_context->term_table_dir);

    printf("Using access code: %s\n",
           phone_num_to_string(access_code_str, sizeof(access_code_str), mm_context->access_code,
                               sizeof(mm_context->access_code)));
    printf("Using key card number: %s\n",
        phone_num_to_string(key_card_number_str, sizeof(key_card_number_str), mm_context->key_card_number,
            sizeof(mm_context->key_card_number)));

    printf("Manager Inter-packet Tx gap: %dms.\n", mm_context->connection.proto.rx_packet_gap * 10);

    if (strnlen(mm_context->ncc_number[0], sizeof(mm_context->ncc_number[0])) >= 1) {
        printf("Using Primary NCC number: %s\n", mm_context->ncc_number[0]);

        if (strnlen(mm_context->ncc_number[1], sizeof(mm_context->ncc_number[0])) == 0) {
            snprintf(mm_context->ncc_number[1], sizeof(mm_context->ncc_number[1]), "%s", mm_context->ncc_number[0]);
        }

        printf("Using Secondary NCC number: %s\n", mm_context->ncc_number[1]);
    } else if (mm_context->connection.proto.use_modem == 1) {
        fprintf(stderr, "Error: -n <NCC Number> must be specified.\n");
        mm_shutdown(mm_context);
        return(-EINVAL);
    }

    if (baudrate < 1200) {
        fprintf(stderr, "Error: baud rate must be 1200 bps or faster.\n");
        return(-EINVAL);
    }
    printf("Baud Rate: %d\n", baudrate);

    mm_context->telco.id[0] = 'V';
    mm_context->telco.id[1] = 'Z';
    mm_context->telco.region_code[0] = 'U';
    mm_context->telco.region_code[1] = 'S';
    mm_context->telco.region_code[2] = '.';

    if ((mm_context->database = mm_open_database("mm_manager.db")) == 0) {
        (void)fprintf(stderr, "mm_manager: error opening database.\n");
        mm_shutdown(mm_context);
        return(-EINVAL);
    }

    status = mm_connection_open(&mm_context->connection, modem_dev, baudrate, mm_context->test_mode);
    if (status != 0) {
        mm_shutdown(mm_context);
        return(status);
    }

    mm_context->cdr_ack_buffer_len = 0;
    printf("Waiting for call from terminal...\n");

    while (manager_running) {

        retries = 0;
        if (mm_connection_wait(&mm_context->connection)) {
            while (proto_connected(&mm_context->connection.proto) && (manager_running) && (retries < 3)) {
                retries++;
                status = process_mm_table(mm_context, &mm_table);
                if (status == PKT_SUCCESS) {
                    retries = 0;
                }
            }

            if (proto_connected(&mm_context->connection.proto)) {
                proto_disconnect(&mm_context->connection.proto);
            }

            mm_time(mm_context->test_mode, &rawtime);
            localtime_r(&rawtime, &ptm);

            printf("\n\n%04d-%02d-%02d %2d:%02d:%02d: Terminal %s: Disconnected.\n\n",
                ptm.tm_year + 1900, ptm.tm_mon + 1, ptm.tm_mday, ptm.tm_hour, ptm.tm_min, ptm.tm_sec,
                mm_context->connection.proto.terminal_id);
        }
    }

    printf("mm_manager: Shutting down.\n");
    mm_shutdown(mm_context);
    return 0;
}

static int mm_shutdown(mm_context_t* context) {
    mm_close_database(context->database);
    mm_connection_close(&context->connection);

    free(context);
    return (0);
}

static int append_to_cdr_ack_buffer(mm_context_t *context, uint8_t *buffer, uint8_t length) {
    if ((size_t)context->cdr_ack_buffer_len + length > sizeof(context->cdr_ack_buffer)) {
        printf("ERROR: %s: cdr_ack_buffer_len exceeded.\n", __func__);
        return -EOVERFLOW;
    }

    memcpy(&context->cdr_ack_buffer[context->cdr_ack_buffer_len], buffer, length);
    context->cdr_ack_buffer_len += length;

    return 0;
}

static int process_mm_table(mm_context_t* context, mm_table_t* table) {
    mm_packet_t* pkt = &table->pkt;
    uint8_t  ack_payload[PKT_TABLE_DATA_LEN_MAX] = { 0 };
    uint8_t* pack_payload = ack_payload;
    char     terminal_id[11];   /* The terminal's phone number */
    char     timestamp_str[20];
    char     timestamp2_str[20];
    uint8_t* ppayload;
    int      reply_length = 0;
    uint8_t  table_download_pending = 0;
    uint8_t  status;

    status = receive_mm_table(&context->connection.proto, table);

    if (status != 0) return status;

    phone_num_to_string(terminal_id, sizeof(terminal_id), pkt->payload, PKT_TABLE_ID_OFFSET);
    ppayload = pkt->payload + PKT_TABLE_ID_OFFSET;

    while (ppayload < pkt->payload + pkt->payload_len) {
        table->table_id = *ppayload;

        if (context->debuglevel > 1) {
            printf("\n\tTerminal ID %s: Processing Table ID %d (0x%02x) %s\n",
                   terminal_id,
                   table->table_id,
                   table->table_id,
                   table_to_string(table->table_id));
        }

        switch (table->table_id) {
            case DLOG_MT_TIME_SYNC_REQ: {
                time_t rawtime;
                struct tm ptm = { 0 };
                dlog_mt_time_sync_t* time_sync_response = (dlog_mt_time_sync_t*)pack_payload;

                ppayload += sizeof(dlog_mt_time_sync_req_t);

                time_sync_response->id = DLOG_MT_TIME_SYNC;

                mm_time(context->test_mode, &rawtime);
                localtime_r(&rawtime, &ptm);

                time_sync_response->year  = (ptm.tm_year & 0xff);      /* Fill current years since 1900 */
                time_sync_response->month = ((ptm.tm_mon + 1) & 0xff); /* Fill current month (1-12) */
                time_sync_response->day   = (ptm.tm_mday & 0xff);      /* Fill current day (1-31) */
                time_sync_response->hour  = (ptm.tm_hour & 0xff);      /* Fill current hour (0-23) */
                time_sync_response->min   = (ptm.tm_min & 0xff);       /* Fill current minute (0-59) */
                time_sync_response->sec   = (ptm.tm_sec & 0xff);       /* Fill current second (0-59) */
                time_sync_response->wday  = (ptm.tm_wday + 1);         /* Day of week, 1=Sunday ... 7=Saturday */

                printf("\t\tCurrent day/time: %04d-%02d-%02d / %2d:%02d:%02d\n",
                    time_sync_response->year + 1900,
                    time_sync_response->month,
                    time_sync_response->day,
                    time_sync_response->hour,
                    time_sync_response->min,
                    time_sync_response->sec);

                pack_payload += sizeof(dlog_mt_time_sync_t);
                *pack_payload++ = DLOG_MT_END_DATA;
                break;
            }
            case DLOG_MT_ATN_REQ_TAB_UPD: {
                ppayload++;
                context->terminal_upd_reason = *ppayload++;
                printf("\t\tTerminal %s requests table update. Reason: 0x%02x [%s%s%s%s%s]\n\n",
                       terminal_id,
                       context->terminal_upd_reason,
                       context->terminal_upd_reason & TTBLREQ_CRAFT_FORCE_DL ? "Force Download, " : "",
                       context->terminal_upd_reason & TTBLREQ_CRAFT_INSTALL  ? "Install, " : "",
                       context->terminal_upd_reason & TTBLREQ_LOST_MEMORY    ? "Lost Memory, " : "",
                       context->terminal_upd_reason & TTBLREQ_PWR_LOST_ON_DL ? "Power Lost on Download, " : "",
                       context->terminal_upd_reason & TTBLREQ_CASHBOX_STATUS ? "Cashbox Status Request" : "");

                /* Send DLOG_MT_TABLE_UPD */
                *pack_payload++ = DLOG_MT_TABLE_UPD;

                /* Send cash box status if requested by terminal */
                if (context->terminal_upd_reason & TTBLREQ_CASHBOX_STATUS) {
                    printf("\tSend DLOG_MT_CASH_BOX_STATUS table as requested by terminal.\n\t");

                    mm_acct_load_TCASHST(context->database, terminal_id, (cashbox_status_univ_t*)pack_payload);
                    pack_payload += sizeof(cashbox_status_univ_t);
                }

                table_download_pending = 1;
                break;
            }
            case DLOG_MT_ALARM: {
                dlog_mt_alarm_t *alarm = (dlog_mt_alarm_t *)ppayload;

                ppayload += sizeof(dlog_mt_alarm_t);

                *pack_payload++ = DLOG_MT_ALARM_ACK;
                *pack_payload++ = alarm->alarm_id;

                mm_acct_save_TALARM(context->database, &context->telco, terminal_id, alarm);

                break;
            }
            case DLOG_MT_MAINT_REQ: {
                dlog_mt_maint_req_t *maint = (dlog_mt_maint_req_t *)ppayload;;
                ppayload += sizeof(dlog_mt_maint_req_t);

                mm_acct_save_TOPCODE(context->database, &context->telco, terminal_id, maint);

                *pack_payload++ = DLOG_MT_MAINT_ACK;
                *pack_payload++ = maint->type & 0xFF;
                *pack_payload++ = (maint->type >> 8) & 0xFF;
                break;
            }
            case DLOG_MT_CALL_DETAILS: {
                dlog_mt_call_details_t *cdr = (dlog_mt_call_details_t *)ppayload;
                uint8_t cdr_ack_buf[3] = { 0 };

                ppayload += sizeof(dlog_mt_call_details_t);

                mm_acct_save_TCDR(context->database, &context->telco, terminal_id, cdr);

                cdr_ack_buf[0] = DLOG_MT_CDR_DETAILS_ACK;
                cdr_ack_buf[1] = cdr->seq & 0xFF;
                cdr_ack_buf[2] = (cdr->seq >> 8) & 0xFF;

                /* If terminal is transferring multiple tables, queue the CDR response for later, after receiving DLOG_MT_END_DATA */
                if (context->trans_data_in_progress == 1) {
                    append_to_cdr_ack_buffer(context, cdr_ack_buf, sizeof(cdr_ack_buf));
                } else {
                    /* If receiving a CDR as part of a credit card auth, etc, send the CDR ack immediately. */
                    memcpy(pack_payload, cdr_ack_buf, sizeof(cdr_ack_buf));
                    pack_payload += sizeof(cdr_ack_buf);
                }
                break;
            }
            case DLOG_MT_ATN_REQ_CDR_UPL: {
                ppayload++; /* Skip over table ID. */

                /* Not sure what the cdr_req_type is, just swallow it. */
                uint8_t cdr_req_type = *ppayload++;
                printf("\t\tDLOG_MT_ATN_REQ_CDR_UPL, cdr_req_type=%02x (0x%02x)\n", cdr_req_type, cdr_req_type);

                *pack_payload++                 = DLOG_MT_TRANS_DATA;
                context->trans_data_in_progress = 1;
                break;
            }
            case DLOG_MT_CASH_BOX_COLLECTION: {
                dlog_mt_cash_box_collection_t *cash_box_collection = (dlog_mt_cash_box_collection_t *)ppayload;

                ppayload += sizeof(dlog_mt_cash_box_collection_t);

                mm_acct_save_TCOLLST(context->database, &context->telco, terminal_id, cash_box_collection);
                *pack_payload++ = DLOG_MT_END_DATA;
                break;
            }
            case DLOG_MT_TERM_STATUS: {
                dlog_mt_term_status_t *dlog_mt_term_status = (dlog_mt_term_status_t *)ppayload;

                ppayload += sizeof(dlog_mt_term_status_t);

                mm_acct_save_TSTATUS(context->database, &context->telco, terminal_id, dlog_mt_term_status);
                break;
            }
            case DLOG_MT_TERM_ERR_REP: {
                printf("\t\tTerminal %s DLOG_MT_TERM_ERR_REP\n\n", terminal_id);

                ppayload += 97;
                break;
            }
            case DLOG_MT_SW_VERSION: {
                dlog_mt_sw_version_t *dlog_mt_sw_version = (dlog_mt_sw_version_t *)ppayload;

                ppayload += sizeof(dlog_mt_sw_version_t);

                mm_acct_save_TSWVERS(context->database, &context->telco, terminal_id, dlog_mt_sw_version, &context->terminal_type);
                break;
            }
            case DLOG_MT_CASH_BOX_STATUS: {
                mm_acct_save_TCASHST(context->database, &context->telco, terminal_id, (cashbox_status_univ_t*)ppayload);

                ppayload += sizeof(cashbox_status_univ_t);
                break;
            }
            case DLOG_MT_PERF_STATS_MSG: {
                dlog_mt_perf_stats_record_t *perf_stats = (dlog_mt_perf_stats_record_t *)ppayload;
                ppayload += sizeof(dlog_mt_perf_stats_record_t);

                mm_acct_save_TPERFST(context->database, &context->telco, terminal_id, perf_stats);
                break;
            }
            case DLOG_MT_CALL_IN: {
                printf("\tDLOG_MT_CALL_IN: Terminal: %s\n", terminal_id);
                ppayload += sizeof(dlog_mt_call_in_t);
                *pack_payload++                 = DLOG_MT_TRANS_DATA;
//                context->terminal_upd_reason |= TTBLREQ_CRAFT_FORCE_DL;
//                table_download_pending = 1;
                context->trans_data_in_progress = 1;
                break;
            }
            case DLOG_MT_CALL_BACK: {
                printf("\tDLOG_MT_CALL_BACK: Terminal: %s\n", terminal_id);
                ppayload += sizeof(dlog_mt_call_back_t);
                *pack_payload++                 = DLOG_MT_TRANS_DATA;
                context->trans_data_in_progress = 1;
                break;
            }
            case DLOG_MT_CARRIER_CALL_STATS:
            {
                dlog_mt_carrier_call_stats_t *carr_stats = (dlog_mt_carrier_call_stats_t *)ppayload;
                ppayload += sizeof(dlog_mt_carrier_call_stats_t);
                /* TODO: Convert to database. */
                printf("\t\tCarrier Call Statistics Record: From: %s, to: %s:\n",
                       timestamp_to_string(carr_stats->timestamp,  timestamp_str,  sizeof(timestamp_str)),
                       timestamp_to_string(carr_stats->timestamp2, timestamp2_str, sizeof(timestamp2_str)));

                for (int i = 0; i < 3; i++) {
                    carrier_stats_entry_t *pcarr_stats_entry = &carr_stats->carrier_stats[i];
                    uint32_t k                               = 0;

                    printf("\t\t\tCarrier 0x%02x:", pcarr_stats_entry->carrier_ref);

                    for (int j = 0; j < 29; j++) {
                        k += pcarr_stats_entry->stats[j];
                    }

                    if (k == 0) {
                        printf("\tNo calls.\n");
                    } else {
                        for (int j = 0; j < 29; j++) {
                            if (j % 2 == 0) printf(" |\n\t\t\t\t");
                            printf("| stats[%24s] =%5d\t\t", stats_to_str(j), pcarr_stats_entry->stats[j]);
                        }
                        printf("\n");
                    }
                }
                break;
            }
            case DLOG_MT_CARRIER_STATS_EXP: {
                dlog_mt_carrier_stats_exp_t *carr_stats = (dlog_mt_carrier_stats_exp_t *)ppayload;
                ppayload += sizeof(dlog_mt_carrier_stats_exp_t);

                printf("\t\tExpanded Carrier Statistics: From: %s, to: %s:\n",
                       timestamp_to_string(carr_stats->timestamp,  timestamp_str,  sizeof(timestamp_str)),
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
                        printf("\t\t\t\t%s stats:\t", stats_call_type_to_str(j));

                        for (int i = 0; i < STATS_EXP_PAYMENT_TYPE_MAX; i++) {
                            printf("%d, ", pcarr_stats_entry->stats[j][i]);
                        }
                        printf("\n");
                    }

                    printf("\t\t\t\tOperator Assisted Call Count: %d\n",    pcarr_stats_entry->operator_assist_call_count);
                    printf("\t\t\t\t0+ Call Count: %d\n",                   pcarr_stats_entry->zero_plus_call_count);
                    printf("\t\t\t\tFree Feature B Call Count: %d\n",       pcarr_stats_entry->free_featb_call_count);
                    printf("\t\t\t\tDirectory Assistance Call Count: %d\n", pcarr_stats_entry->directory_assist_call_count);
                    printf("\t\t\t\tTotal Call duration: %u\n",             pcarr_stats_entry->total_call_duration);
                    printf("\t\t\t\tTotal Insert Mode Calls: %d\n",         pcarr_stats_entry->total_insert_mode_calls);
                    printf("\t\t\t\tTotal Manual Mode Calls: %d\n",         pcarr_stats_entry->total_manual_mode_calls);
                }
                break;
            }
            case DLOG_MT_SUMMARY_CALL_STATS: {
                dlog_mt_summary_call_stats_t *summary_call_stats = (dlog_mt_summary_call_stats_t *)ppayload;
                ppayload += sizeof(dlog_mt_summary_call_stats_t);

                mm_acct_save_TCALLST(context->database, &context->telco, terminal_id, summary_call_stats);
                break;
            }
            case DLOG_MT_RATE_REQUEST: {
                char phone_number[21]                        = { 0 };
                char call_type_str[38]                       = { 0 };
                dlog_mt_rate_response_t rate_response        = { 0 };
                dlog_mt_rate_request_t *rate_request = (dlog_mt_rate_request_t *)ppayload;
                ppayload += sizeof(dlog_mt_rate_request_t);

                phone_num_to_string(phone_number, sizeof(phone_number), rate_request->phone_number,
                                    sizeof(rate_request->phone_number));
                call_type_to_string(rate_request->call_type & (~FLAG_CDR_IXL), call_type_str, sizeof(call_type_str));

                printf("\t\tRate request: %s: Phone number: %s, pad=%d, telco_id=%d, pad2=%d, call_type=0x%02x (%s), pad3=%d, rate_type=%d, pad4=%d,%d.\n",
                       timestamp_to_string(rate_request->timestamp, timestamp_str, sizeof(timestamp_str)),
                       phone_number,
                       rate_request->pad,
                       rate_request->telco_id,
                       rate_request->pad2,
                       rate_request->call_type,
                       call_type_str,
                       rate_request->pad3,
                       rate_request->rate_type,
                       rate_request->pad4[0],
                       rate_request->pad4[1]);

                rate_response.id = DLOG_MT_RATE_RESPONSE;
                rate_response.rate.type = (uint8_t)mm_inter_lata;

                if (context->rating_test_mode) {
                    rate_response.rate.initial_period = 60;
                    rate_response.rate.initial_charge = ((phone_number[6] - '0') * 1000) + ((phone_number[7] - '0') * 100) + ((phone_number[8] - '0') * 10) + (phone_number[9] - '0');
                    rate_response.rate.additional_period = 0x00;
                    rate_response.rate.additional_charge = 0x00;
                }
                else {
                    rate_response.rate.initial_period = 240;
                    rate_response.rate.initial_charge = 100;
                    rate_response.rate.additional_period = 60;
                    rate_response.rate.additional_charge = 25;
                }

                printf("\t\tRate response: Rate type: %d (%s), Initial period: %d, Initial charge: %d, Additional Period: %d, Additional Charge: %d\n",
                    rate_response.rate.type,
                    rate_type_to_str(rate_response.rate.type),
                    rate_response.rate.initial_period,
                    rate_response.rate.initial_charge,
                    rate_response.rate.additional_period,
                    rate_response.rate.additional_charge);

                memcpy(pack_payload, &rate_response, sizeof(rate_response));
                pack_payload += sizeof(rate_response);
//#define REQUEST_CALL_BACK_DURING_RATE_REQ
#ifdef REQUEST_CALL_BACK_DURING_RATE_REQ
                {
                    time_t rawtime = { 0 };
                    struct tm ptm = { 0 };
                    dlog_mt_call_back_req_t   call_back_req = { DLOG_MT_CALL_BACK_REQ, 0, 0, 0, 0, 0, 0 };

                    call_back_req.id = DLOG_MT_CALL_BACK_REQ;

                    mm_time(context->test_mode, &rawtime);
                    localtime_r(&rawtime, &ptm);

                    call_back_req.year = (ptm.tm_year & 0xff);    /* Fill current years since 1900 */
                    call_back_req.month = (ptm.tm_mon + 1 & 0xff); /* Fill current month (1-12) */
                    call_back_req.day = (ptm.tm_mday & 0xff);    /* Fill current day (1-31) */
                    call_back_req.hour = (ptm.tm_hour & 0xff);    /* Fill current hour (0-23) */
                    call_back_req.min = ((ptm.tm_min + 2) & 0xff);     /* Fill current minute (0-59) */
                    call_back_req.sec = (ptm.tm_sec & 0xff);     /* Fill current second (0-59) */

                    memcpy(pack_payload, &call_back_req, sizeof(call_back_req));
                    pack_payload += sizeof(dlog_mt_call_back_req_t);

                    printf("\t\tRequest callback at day/time: %04d-%02d-%02d / %2d:%02d:%02d\n",
                        call_back_req.year + 1900,
                        call_back_req.month,
                        call_back_req.day,
                        call_back_req.hour,
                        call_back_req.min,
                        call_back_req.sec);

                }
#endif /* REQUEST_CALL_BACK_DURING_RATE_REQ */

                break;
            }
            case DLOG_MT_FUNF_CARD_AUTH: {
                dlog_mt_auth_resp_code_t  auth_response = { DLOG_MT_AUTH_RESP_CODE, 0 , 0, { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42 }};
                dlog_mt_funf_card_auth_t *auth_request  = (dlog_mt_funf_card_auth_t *)ppayload;
                time_t rawtime;

                mm_time(context->test_mode, &rawtime);
                ppayload += sizeof(dlog_mt_funf_card_auth_t);

                mm_acct_save_TAUTH(context->database, &context->telco, terminal_id, auth_request);

                auth_response.resp_code = 0;
                auth_response.auth_code = rawtime;

                printf("\t\tSending auth response: Response code: 0x%02x, Authorization code: %" PRIu64 "\n",
                    auth_response.resp_code,
                    auth_response.auth_code);

                memcpy(pack_payload, &auth_response, sizeof(auth_response));
                pack_payload += sizeof(auth_response);

//#define REQUEST_CALL_BACK_DURING_CARD_AUTH
#ifdef REQUEST_CALL_BACK_DURING_CARD_AUTH
                {
                    time_t rawtime = { 0 };
                    struct tm ptm = { 0 };
                    dlog_mt_call_back_req_t   call_back_req = { DLOG_MT_CALL_BACK_REQ, 0, 0, 0, 0, 0, 0 };

                    call_back_req.id = DLOG_MT_CALL_BACK_REQ;

                    mm_time(context->test_mode, &rawtime);
                    localtime_r(&rawtime, &ptm);

                    call_back_req.year = (ptm.tm_year & 0xff);    /* Fill current years since 1900 */
                    call_back_req.month = (ptm.tm_mon + 1 & 0xff); /* Fill current month (1-12) */
                    call_back_req.day = (ptm.tm_mday & 0xff);    /* Fill current day (1-31) */
                    call_back_req.hour = (ptm.tm_hour & 0xff);    /* Fill current hour (0-23) */
                    call_back_req.min = ((ptm.tm_min + 1) & 0xff);     /* Fill current minute (0-59) */
                    call_back_req.sec = (ptm.tm_sec & 0xff);     /* Fill current second (0-59) */

                    memcpy(pack_payload, &call_back_req, sizeof(call_back_req));
                    pack_payload += sizeof(dlog_mt_call_back_req_t);
                }
#endif /* REQUEST_CALL_BACK_DURING_CARD_AUTH */
                break;
            }
            case DLOG_MT_END_DATA:
                ppayload += sizeof(dlog_mt_end_data_t);
                context->trans_data_in_progress = 0;

                *pack_payload++ = DLOG_MT_END_DATA;

                if (context->cdr_ack_buffer_len > 0) {
                    memcpy(pack_payload, context->cdr_ack_buffer, context->cdr_ack_buffer_len);
                    pack_payload += context->cdr_ack_buffer_len;
                    printf("Appending CDR ACKs to DLOG_MT_END_DATA.\n");
                    context->cdr_ack_buffer_len = 0;
                } else {
                    printf("Sending DLOG_MT_END_DATA.\n");
                }

                break;
            case DLOG_MT_TABLE_UPD_ACK:
                printf("\tDLOG_MT_TABLE_UPD_ACK for table 0x%02x.\n", *ppayload);
                ppayload+=2;
                *pack_payload++ = DLOG_MT_TRANS_DATA;
                break;
            default:
                fprintf(stderr, "Error: * * * Unhandled table %d (0x%02x)", table->table_id, table->table_id);
                ppayload++;
                break;
        }
    }

    reply_length = (int)(pack_payload - ack_payload);

    if (reply_length > 0) {
        send_mm_table(&context->connection.proto, ack_payload, (int)(pack_payload - ack_payload));
    }

    if (table_download_pending == 1) {
        mm_download_tables(context, terminal_id);
    }

    return 0;
}

static int mm_download_tables(mm_context_t *context, char *terminal_id) {
    int      table_index;
    int      status = 0;
    size_t   table_len;
    uint8_t *table_buffer;
    uint8_t *table_list = table_list_mtr_2x;
    uint8_t  table_id;
    uint8_t  term_model = term_type_to_model(context->terminal_type);

    switch (term_type_to_mtr(context->terminal_type)) {
    case MTR_2_X:
        table_list = table_list_mtr_2x;
        break;
    case MTR_1_20:
        table_list = table_list_mtr_120;
        break;
    case MTR_1_13:
    case MTR_1_11:
    case MTR_1_10:
    case MTR_1_9:
        table_list = table_list_mtr19;
        break;
    case MTR_1_7_INTL:
        table_list = table_list_mtr17_intl;
        break;
    case MTR_1_7:
    case MTR_1_6:
        table_list = table_list_mtr17;
        break;
    default:
        fprintf(stderr, "%s: Error: Unknown terminal type %d, defaulting to MTR 1.7\n", __func__, context->terminal_type);
        table_list = table_list_mtr17;
        break;
    }

    for (table_index = 0; (table_id = table_list[table_index]) > 0; table_index++) {
        /* Abort table download if manager is shutting down. */
        if (!manager_running) break;
        if (!proto_connected(&context->connection.proto)) break;

        /* Skip DLOG_MT_CARD_TABLE, DLOG_MT_CARD_TABLE_EXP if the terminal is coin-only. */
        if (term_model == TERM_COIN_BASIC) {
            switch (table_id) {
            case DLOG_MT_CARD_TABLE:
            case DLOG_MT_CARD_TABLE_EXP:
                continue;
            default:
                break;
            }
        }
        else if (((term_model == TERM_CARD) || (term_model == TERM_DESK)) && (table_id == DLOG_MT_COIN_VAL_TABLE)) {
            /* Skip DLOG_MT_COIN_VAL_TABLE for card-only terminals */
            continue;
        }

        /* If -s was specified, only download mandatory tables */
        if (context->minimal_table_set == 1) {
            switch (table_id) {
            case DLOG_MT_NCC_TERM_PARAMS:
            case DLOG_MT_CARD_TABLE:
            case DLOG_MT_CARRIER_TABLE:
            case DLOG_MT_CALLSCRN_UNIVERSAL:
            case DLOG_MT_FCONFIG_OPTS:
            case DLOG_MT_INSTALL_PARAMS:
            case DLOG_MT_COIN_VAL_TABLE:
            case DLOG_MT_NUM_PLAN_TABLE:
            case DLOG_MT_SPARE_TABLE:
            case DLOG_MT_RATE_TABLE:
            case DLOG_MT_CALL_SCREEN_LIST:
            case DLOG_MT_SCARD_PARM_TABLE:
            case DLOG_MT_CARD_TABLE_EXP:
            case DLOG_MT_CARRIER_TABLE_EXP:
            case DLOG_MT_NPA_NXX_TABLE_1:
            case DLOG_MT_COMP_LCD_TABLE_1:
            case DLOG_MT_LCD_TABLE_1:
            case DLOG_MT_END_DATA:
                break;
            default: /* Skip tables that are not mandatory */
                continue;
            }
        }

        switch (table_id) {
            case DLOG_MT_INSTALL_PARAMS:
                generate_install_parameters(context, &table_buffer, &table_len);
                break;
            case DLOG_MT_CALL_IN_PARMS:
                generate_call_in_parameters(context, &table_buffer, &table_len);
                break;
            case DLOG_MT_NCC_TERM_PARAMS:
                if (term_type_to_mtr(context->terminal_type) <= MTR_1_13) {
                    generate_term_access_parameters_mtr1(context, terminal_id, &table_buffer, &table_len);
                } else {
                    generate_term_access_parameters(context, terminal_id, &table_buffer, &table_len);
                }
                break;
            case DLOG_MT_CALL_STAT_PARMS:
                generate_call_stat_parameters(context, &table_buffer, &table_len);
                break;
            case DLOG_MT_COMM_STAT_PARMS:
                generate_comm_stat_parameters(context, &table_buffer, &table_len);
                break;
            case DLOG_MT_END_DATA:
                generate_dlog_mt_end_data(context, &table_buffer, &table_len);
                break;
            case DLOG_MT_CASH_BOX_STATUS:
            {
                cashbox_status_univ_t *pcashbox_status = { 0 };
                pcashbox_status = (cashbox_status_univ_t *)calloc(1, sizeof(cashbox_status_univ_t));
                table_buffer = (uint8_t*)pcashbox_status;
                if (table_buffer == NULL) {
                    fprintf(stderr, "%s: Error: failed to allocate %zu bytes.\n", __func__, sizeof(cashbox_status_univ_t));
                    return -ENOMEM;
                }
                mm_acct_load_TCASHST(context->database, terminal_id, (cashbox_status_univ_t *)table_buffer);
                table_len = sizeof(cashbox_status_univ_t);
                break;
            }
            default:
                printf("\t");
                /* For Craft Force Download, only download tables that are newer,
                 * unless the terminal lost its memory or the the "-c" option was
                 * selected.
                 */
                if ((context->complete_download == FALSE) &&
                    (context->terminal_upd_reason & TTBLREQ_CRAFT_FORCE_DL) &&
                    !(context->terminal_upd_reason & TTBLREQ_LOST_MEMORY) &&
                    !(context->terminal_upd_reason & TTBLREQ_PWR_LOST_ON_DL)) {
                    if (check_mm_table_is_newer(context, terminal_id, table_id) != 0) {
                        table_buffer = NULL;
                        continue;
                    }
                }

                status = load_mm_table(context, terminal_id, table_id, &table_buffer, &table_len);

                if (status != 0) {
                    if (table_id == DLOG_MT_USER_IF_PARMS) { /* Can't load DLOG_MT_USER_IF_PARMS, generate it. */
                        generate_user_if_parameters(context, &table_buffer, &table_len);
                    }
                    else { /* If table can't be loaded, continue to the next. */
                        if (table_buffer != NULL) free(table_buffer);
                        table_buffer = NULL;
                        continue;
                    }
                }
                break;
        }

        /* Update DLOG_MT_FCONFIG_OPTS based on terminal type. */
        if (table_id == DLOG_MT_FCONFIG_OPTS) {
            ((dlog_mt_fconfig_opts_t*)table_buffer)->term_type = term_model & 0x0F;
        }

        status = send_mm_table(&context->connection.proto, table_buffer, table_len);

        if (status == PKT_SUCCESS) {
            /* For all tables except END_OF_DATA, expect a table ACK. */
            if (table_list[table_index] != DLOG_MT_END_DATA) {
                status = wait_for_table_ack(&context->connection.proto, table_buffer[0]);
            }
        }

        free(table_buffer);
        table_buffer = NULL;

    }

    if (proto_connected(&context->connection.proto)) {
        /* Update table download time. */
        update_terminal_download_time(context, terminal_id);
    } else {
        printf("%s: Download failed.\n", __func__);
    }

    return status;
}

static int update_terminal_download_time(mm_context_t *context, char *terminal_id) {
    FILE *stream;
    char  fname[TABLE_PATH_MAX_LEN + 1];
    char  date[100];
    time_t rawtime;
    struct tm ptm = { 0 };

    if (terminal_id[0] != '\0') {
        snprintf(fname, sizeof(fname), "%s/%s/table_update.log", context->term_table_dir, terminal_id);
    } else {
        return -EINVAL;
    }

    create_terminal_specific_directory(context->term_table_dir, terminal_id);

    mm_time(context->test_mode, &rawtime);
    localtime_r(&rawtime, &ptm);
    strftime(date, 99, "%Y-%m-%d %H:%M:%S", &ptm);

    if (!(stream = fopen(fname, "a+"))) {
        fprintf(stderr, "%s: Error: Could not open '%s'.\n", __func__, fname);
        return -ENOENT;
    }

    printf("%s: Terminal %s download complete.\n", date, terminal_id);
    fprintf(stream, "%s: Terminal %s download complete.\n", date, terminal_id);
    fclose(stream);

    return 0;
}

static int check_mm_table_is_newer(mm_context_t *context, char *terminal_id, uint8_t table_id) {
    char  fname[TABLE_PATH_MAX_LEN];
    char  download_time_fname[TABLE_PATH_MAX_LEN + 1];
    struct stat table_mtime_attr;
    struct stat last_download_time_attr;

    char  last_download_date[100];
    char  table_mtime_date[100];

    struct tm ptm = { 0 };

    if (terminal_id[0] != '\0') {
        snprintf(fname, sizeof(fname), "%s/%s/mm_table_%02x.bin", context->term_table_dir, terminal_id, table_id);
        snprintf(download_time_fname, sizeof(download_time_fname), "%s/%s/table_update.log", context->term_table_dir, terminal_id);
        if (stat(fname, &table_mtime_attr) == -1) {
            snprintf(fname, sizeof(fname), "%s/mm_table_%02x.bin", context->default_table_dir, table_id);
            if (stat(fname, &table_mtime_attr) == -1) {
                table_mtime_attr.st_mtime = 0;
            }
        }

        if (stat(download_time_fname, &last_download_time_attr) == -1) {
            last_download_time_attr.st_mtime = 0;
        }
    } else {
        table_mtime_attr.st_mtime = 0;
        last_download_time_attr.st_mtime = 0;
    }

    localtime_r(&last_download_time_attr.st_mtime, &ptm);
    strftime(last_download_date, 99, "%Y-%m-%d %H:%M:%S", &ptm);

    localtime_r(&table_mtime_attr.st_mtime, &ptm);
    strftime(table_mtime_date, 99, "%Y-%m-%d %H:%M:%S", &ptm);

    if (table_mtime_attr.st_mtime < last_download_time_attr.st_mtime) {
        printf("Skipping download of table %d: last downloaded: %s, mtime: %s.\n",
            table_id,
            last_download_date,
            table_mtime_date);
            return -1;
    }
    return 0;
}

static int load_mm_table(mm_context_t *context, char *terminal_id, uint8_t table_id, uint8_t **buffer, size_t *len) {
    FILE *stream;
    char  fname[TABLE_PATH_MAX_LEN];
    uint32_t size;
    uint8_t *bufp;
    uint8_t  term_model = term_type_to_model(context->terminal_type);

    if (terminal_id[0] != '\0') {
        snprintf(fname, sizeof(fname), "%s/%s/mm_table_%02x.bin", context->term_table_dir, terminal_id, table_id);
    } else {
        snprintf(fname, sizeof(fname), "%s/mm_table_%02x.bin", context->default_table_dir, table_id);
    }

    /* Try to load terminal-specific table first. */
    if (!(stream = fopen(fname, "rb"))) {

        /* No terminal-specific table, try based on model. */
        switch (term_model) {
        case TERM_CARD:
            snprintf(fname, sizeof(fname), "%s/card_only/mm_table_%02x.bin", context->term_table_dir, table_id);
            break;
        case TERM_DESK:
            snprintf(fname, sizeof(fname), "%s/desk/mm_table_%02x.bin", context->term_table_dir, table_id);
            break;
        case TERM_COIN_BASIC:
            snprintf(fname, sizeof(fname), "%s/coin/mm_table_%02x.bin", context->term_table_dir, table_id);
            break;
        case TERM_INMATE:
            snprintf(fname, sizeof(fname), "%s/inmate/mm_table_%02x.bin", context->term_table_dir, table_id);
            break;
        case TERM_MULTIPAY:
        default:
            snprintf(fname, sizeof(fname), "%s/multipay/mm_table_%02x.bin", context->term_table_dir, table_id);
            break;
        }

        if (!(stream = fopen(fname, "rb"))) {
            /* No model-specific table, fall back to default table directory. */
            snprintf(fname, sizeof(fname), "%s/mm_table_%02x.bin", context->default_table_dir, table_id);

            if (!(stream = fopen(fname, "rb"))) {
                printf("Could not load table %d from %s.\n", table_id, fname);
                *buffer = NULL;
                return -1;
            }
        }
    }

    fseek(stream, 0, SEEK_END);
    size = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    size++;  // Make room for table ID.

    if ((table_id == DLOG_MT_CALL_SCREEN_LIST) &&
        ((term_type_to_mtr(context->terminal_type) >= MTR_1_9) && (term_type_to_mtr(context->terminal_type) < MTR_1_20))) {
        if (size == 3061) {
            size += 340;    /* Pad 180-entry Call Screen List to 200-entries. */
        }
    }

    *buffer = (uint8_t *)calloc(size, sizeof(uint8_t));
    fflush(stdout);

    if (*buffer == NULL) {
        fprintf(stderr, "%s: Error: failed to allocate %u bytes for table %d\n", __func__, size, table_id);
        fclose(stream);
        return -ENOMEM;
    }

    bufp    = *buffer;
    *bufp++ = table_id;

    *len = 1;

    while (1) {
        uint8_t c = (uint8_t)fgetc(stream);

        if (feof(stream)) {
            break;
        }
        *bufp++ = c;
        *len    = *len + 1;

        if (*len > size) {
            fclose(stream);
            free(*buffer);
            *buffer = NULL;
            return -1;
        }
    }

    *len = size;

    printf("Loaded table ID %d (0x%02x) from %s (%zu bytes).\n", table_id, table_id, fname, *len - 1);
    fclose(stream);

    return 0;
}


static void generate_install_parameters(mm_context_t* context, uint8_t** buffer, size_t* len) {
    dlog_mt_install_params_t* pinstall_params;
    uint8_t* pbuffer;

    *len = sizeof(dlog_mt_install_params_t);
    pbuffer = (uint8_t*)calloc(1, *len);

    if (pbuffer == NULL) {
        fprintf(stderr, "%s: Error allocating %zu bytes of memory\n", __func__, *len);
        mm_shutdown(context);
        exit(-ENOMEM);
    }

    pinstall_params = (dlog_mt_install_params_t*)pbuffer;

    printf("\nGenerating Install Parameters (INSTSV) table:\n");

    pinstall_params->id = DLOG_MT_INSTALL_PARAMS;

    memcpy(pinstall_params->access_code, context->access_code, sizeof(pinstall_params->access_code));
    memcpy(pinstall_params->key_card_number, context->key_card_number, sizeof(pinstall_params->key_card_number));
    pinstall_params->tx_packet_delay = 10;
    pinstall_params->rx_packet_gap   = context->connection.proto.rx_packet_gap;
    pinstall_params->retries_until_oos = 40;
    pinstall_params->coinbox_lock_timeout = 150;
    pinstall_params->predial_string[0] = 0x0a;
    pinstall_params->predial_string_alt[0] = 0x0a;

    print_instsv_table(pinstall_params);

    *buffer = pbuffer;
}

static void generate_term_access_parameters(mm_context_t *context, char *terminal_id, uint8_t **buffer, size_t *len) {
    int i;
    dlog_mt_ncc_term_params_t *pncc_term_params;

    *len    = sizeof(dlog_mt_ncc_term_params_t);
    pncc_term_params = (dlog_mt_ncc_term_params_t *)(uint8_t*)calloc(1, *len);

    if (pncc_term_params == NULL) {
        fprintf(stderr, "%s: Error allocating %zu bytes of memory\n", __func__, *len);
        mm_shutdown(context);
        exit(-ENOMEM);
    }

    printf("\nGenerating Terminal Access Parameters table:\n" \
           "\t  Terminal ID: %s\n", terminal_id);

    pncc_term_params->id = DLOG_MT_NCC_TERM_PARAMS;

    // Rewrite table with our Terminal ID (phone number)
    for (i = 0; i < PKT_TABLE_ID_OFFSET; i++) {
        pncc_term_params->terminal_id[i]  = (terminal_id[i * 2] - '0') << 4;
        pncc_term_params->terminal_id[i] |= (terminal_id[i * 2 + 1] - '0');
    }

    // Rewrite table with Primary NCC phone number
    printf("\t  Primary NCC: %s\n", context->ncc_number[0]);
    string_to_bcd_a(context->ncc_number[0], pncc_term_params->pri_ncc_number, sizeof(pncc_term_params->pri_ncc_number));

    // Rewrite table with Secondary NCC phone number, if provided.
    if (strnlen(context->ncc_number[1], sizeof(context->ncc_number[1])) > 0) {
        printf("\tSecondary NCC: %s\n", context->ncc_number[1]);
        string_to_bcd_a(context->ncc_number[1], pncc_term_params->sec_ncc_number, sizeof(pncc_term_params->sec_ncc_number));
    }

    *buffer = (uint8_t *)pncc_term_params;
}

static void generate_term_access_parameters_mtr1(mm_context_t *context, char *terminal_id, uint8_t **buffer, size_t *len) {
    int i;
    dlog_mt_ncc_term_params_mtr1_t *pncc_term_params;
    uint8_t *pbuffer;

    *len    = sizeof(dlog_mt_ncc_term_params_mtr1_t);
    pbuffer = (uint8_t*)calloc(1, *len);

    if (pbuffer == NULL) {
        fprintf(stderr, "%s: Error allocating %zu bytes of memory\n", __func__, *len);
        mm_shutdown(context);
        exit(-ENOMEM);
    }

    pncc_term_params = (dlog_mt_ncc_term_params_mtr1_t *)pbuffer;

    printf("\nGenerating Terminal Access Parameters table (MTR 1.x):\n" \
           "\t  Terminal ID: %s\n", terminal_id);

    pncc_term_params->id = DLOG_MT_NCC_TERM_PARAMS;
    // Rewrite table with our Terminal ID (phone number)
    for (i = 0; i < PKT_TABLE_ID_OFFSET; i++) {
        pncc_term_params->terminal_id[i]  = (terminal_id[i * 2] - '0') << 4;
        pncc_term_params->terminal_id[i] |= (terminal_id[i * 2 + 1] - '0');
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

static void generate_call_in_parameters(mm_context_t *context, uint8_t **buffer, size_t *len) {
    dlog_mt_call_in_params_t *pcall_in_params;
    uint8_t  *pbuffer;
    time_t    rawtime;
    struct tm ptm = { 0 };
    uint8_t   call_in_hour;

    *len    = sizeof(dlog_mt_call_in_params_t);
    pbuffer = (uint8_t*)calloc(1, *len);

    if (pbuffer == NULL) {
        fprintf(stderr, "%s: Error allocating %zu bytes of memory\n", __func__, *len);
        mm_shutdown(context);
        exit(-ENOMEM);
    }

    pcall_in_params = (dlog_mt_call_in_params_t *)pbuffer;

    printf("\nGenerating Call-In table:\n");

    pcall_in_params->id = DLOG_MT_CALL_IN_PARMS;

    mm_time(context->test_mode, &rawtime);
    localtime_r(&rawtime, &ptm);

    /* Interestingly, the terminal will call in starting at the call-in time, and continue
     * calling in at intervals specified, up until midnight.  After that, the terminal will
     * not call in until the call-in time the following day.  Since we want to call in twice
     * a day, set the call-in hour to a time in the AM, so the subsequent call 12 hours later
     * will be in the PM of the same day.
     */
    call_in_hour = (ptm.tm_hour & 0xff);

    if (call_in_hour >= 12) { /* If after noon, set back to the AM. */
        call_in_hour -= 12;
    }

    pcall_in_params->call_in_start_date[0]      = (ptm.tm_year & 0xff);       /* Call-in start YY */
    pcall_in_params->call_in_start_date[1]      = ((ptm.tm_mon + 1) & 0xff);  /* Call in start MM */
    pcall_in_params->call_in_start_date[2]      = (ptm.tm_mday & 0xff);       /* Call in start DD */
    pcall_in_params->call_in_start_time[0]      = call_in_hour;               /* Call-in start HH */
    pcall_in_params->call_in_start_time[1]      = (ptm.tm_min & 0xff);        /* Call-in start MM */
    pcall_in_params->call_in_start_time[2]      = (ptm.tm_sec & 0xff);        /* Call-in start SS */
    pcall_in_params->call_in_interval[0]        = 0;                          /* Call-in inteval DD */
    pcall_in_params->call_in_interval[1]        = 12;                         /* Call-in inteval HH */
    pcall_in_params->call_in_interval[2]        = 0;                          /* Call-in inteval MM */
    pcall_in_params->call_back_retry_time[0]    = 15;                         /* Call-back retry time MM */
    pcall_in_params->call_back_retry_time[1]    = 0;                          /* Call-back retry time SS */
    pcall_in_params->cdr_threshold              = 30;                         /* Indicates the number of CDRs that the terminal will store before automatically calling in to the Millennium Manager to upload them. (Range: 1-50) */
    pcall_in_params->call_in_expiration_date[0] = ((ptm.tm_year + 1) & 0xff); /* Expiration timestamp YY */
    pcall_in_params->call_in_expiration_date[1] = ((ptm.tm_mon + 1) & 0xff);  /* Expiration timestamp MM */
    pcall_in_params->call_in_expiration_date[2] = (ptm.tm_mday & 0xff);       /* Expiration timestamp DD */
    pcall_in_params->call_in_expiration_time[0] = 2;                          /* Expiration timestamp HH */
    pcall_in_params->call_in_expiration_time[1] = 0;                          /* Expiration timestamp MM */
    pcall_in_params->call_in_expiration_time[2] = 0;                          /* Expiration timestamp SS */
    pcall_in_params->unknown[0]                 = 0;
    pcall_in_params->unknown[1]                 = 0;

    print_call_in_params_table(pcall_in_params);

    *buffer = pbuffer;
}

static void generate_call_stat_parameters(mm_context_t *context, uint8_t **buffer, size_t *len) {
    dlog_mt_call_stat_params_t *pcall_stat_params;
    uint8_t *pbuffer;

    *len    = sizeof(dlog_mt_call_stat_params_t);
    pbuffer = (uint8_t*)calloc(1, *len);

    if (pbuffer == NULL) {
        fprintf(stderr, "%s: Error allocating %zu bytes of memory\n", __func__, *len);
        mm_shutdown(context);
        exit(-ENOMEM);
    }

    pcall_stat_params = (dlog_mt_call_stat_params_t *)pbuffer;

    printf("\nGenerating Call Stat Parameters table:\n");
    pcall_stat_params->id = DLOG_MT_CALL_STAT_PARMS;
    pcall_stat_params->callstats_start_time[0]  = 0; /* HH */
    pcall_stat_params->callstats_start_time[1]  = 0; /* MM */
    pcall_stat_params->callstats_duration       = 1; /* Indicates the number of days over which call statistics will be accumulated. */
    pcall_stat_params->callstats_threshold      = 1;
    pcall_stat_params->timestamp[0][0]          = 0; /* HH */
    pcall_stat_params->timestamp[0][1]          = 0; /* MM */
    pcall_stat_params->timestamp[1][0]          = 0; /* HH */
    pcall_stat_params->timestamp[1][1]          = 0; /* MM */
    pcall_stat_params->timestamp[2][0]          = 0; /* HH */
    pcall_stat_params->timestamp[2][1]          = 0; /* MM */
    pcall_stat_params->timestamp[3][0]          = 0; /* HH */
    pcall_stat_params->timestamp[3][1]          = 0; /* MM */
    pcall_stat_params->enable                   = 6;
    pcall_stat_params->cdr_threshold            = 40;
    pcall_stat_params->cdr_start_time[0]        = 0; /* HH */
    pcall_stat_params->cdr_start_time[1]        = 0; /* MM */
    pcall_stat_params->cdr_duration_days        = 255;
    pcall_stat_params->cdr_duration_hours_flags = 0;
    *buffer                                     = pbuffer;

    print_call_stat_params_table(pcall_stat_params);

}

static void generate_comm_stat_parameters(mm_context_t *context, uint8_t **buffer, size_t *len) {
    dlog_mt_comm_stat_params_t *pcomm_stat_params;
    uint8_t *pbuffer;

    *len    = sizeof(dlog_mt_comm_stat_params_t);
    pbuffer = (uint8_t*)calloc(1, *len);

    if (pbuffer == NULL) {
        fprintf(stderr, "%s: Error allocating %zu bytes of memory\n", __func__, *len);
        mm_shutdown(context);
        exit(-ENOMEM);
    }

    pcomm_stat_params = (dlog_mt_comm_stat_params_t *)pbuffer;

    printf("\nGenerating Comm Stat Parameters table:\n");
    pcomm_stat_params->id = DLOG_MT_COMM_STAT_PARMS;
    pcomm_stat_params->co_access_dial_complete       = 100;
    pcomm_stat_params->co_access_dial_complete_int   = 50;
    pcomm_stat_params->dial_complete_carr_detect     = 100;
    pcomm_stat_params->dial_complete_carr_detect_int = 50;
    pcomm_stat_params->carr_detect_first_pac         = 10;
    pcomm_stat_params->carr_detect_first_pac_int     = 5;
    pcomm_stat_params->user_waiting_expect_info      = 600;
    pcomm_stat_params->user_waiting_expect_info_int  = 100;
    pcomm_stat_params->perfstats_threshold           = 1;
    pcomm_stat_params->perfstats_start_time[0]       = 0; /* HH */
    pcomm_stat_params->perfstats_start_time[1]       = 0; /* MM */
    pcomm_stat_params->perfstats_duration            = 1; /* Indicates the number of days over which perf statistics will be accumulated. */
    pcomm_stat_params->perfstats_timestamp[0][0]     = 0; /* HH */
    pcomm_stat_params->perfstats_timestamp[0][1]     = 0; /* MM */
    pcomm_stat_params->perfstats_timestamp[1][0]     = 0; /* HH */
    pcomm_stat_params->perfstats_timestamp[1][1]     = 0; /* MM */
    pcomm_stat_params->perfstats_timestamp[2][0]     = 0; /* HH */
    pcomm_stat_params->perfstats_timestamp[2][1]     = 0; /* MM */
    pcomm_stat_params->perfstats_timestamp[3][0]     = 0; /* HH */
    pcomm_stat_params->perfstats_timestamp[3][1]     = 0; /* MM */
    *buffer                                          = pbuffer;

    print_comm_stat_table(pcomm_stat_params);
}

static void generate_user_if_parameters(mm_context_t *context, uint8_t **buffer, size_t *len) {
    dlog_mt_user_if_params_t *puser_if_params;
    uint8_t *pbuffer;

    *len    = sizeof(dlog_mt_user_if_params_t);
    pbuffer = (uint8_t*)calloc(1, *len);

    if (pbuffer == NULL) {
        fprintf(stderr, "%s: Error allocating %zu bytes of memory\n", __func__, *len);
        mm_shutdown(context);
        exit(-ENOMEM);
    }

    puser_if_params = (dlog_mt_user_if_params_t *)pbuffer;

    printf("\nGenerating User Interface Parameters table:\n");
    puser_if_params->id = DLOG_MT_USER_IF_PARMS;
    puser_if_params->digit_clear_delay               = 450;
    puser_if_params->transient_delay                 = 450;
    puser_if_params->transient_hint_time             = 450;
    puser_if_params->visual_to_voice_delay           = 450;
    puser_if_params->voice_repitition_delay          = 450;
    puser_if_params->no_action_timeout               = 3000;
    puser_if_params->card_validation_timeout         = 4500; /* Change from 30s to 45s */
    puser_if_params->dj_second_string_dtmf_timeout   = 300;
    puser_if_params->spare_timer_b                   = 1100;
    puser_if_params->cp_input_timeout                = 100;
    puser_if_params->language_timeout                = 1000;
    puser_if_params->cfs_timeout                     = 200;
    puser_if_params->called_party_disconnect         = 1200;
    puser_if_params->no_voice_prompt_reps            = 3;
    puser_if_params->accs_digit_timeout              = 450;
    puser_if_params->collect_call_timeout            = 400;
    puser_if_params->bong_tone_timeout               = 300;
    puser_if_params->accs_no_action_timeout          = 450;
    puser_if_params->card_auth_required_timeout      = 4000;
    puser_if_params->rate_request_timeout            = 6000;
    puser_if_params->manual_dial_hold_time           = 50;
    puser_if_params->autodialer_hold_time            = 300;
    puser_if_params->coin_first_warning_time         = 30;
    puser_if_params->coin_second_warning_time        = 5;
    puser_if_params->alternate_bong_tone_timeout     = 200;
    puser_if_params->delay_after_bong_tone           = 125;
    puser_if_params->alternate_delay_after_bong_tone = 125;
    puser_if_params->display_scroll_speed            = 0;
    puser_if_params->aos_bong_tone_timeout           = 600;
    puser_if_params->fgb_aos_second_spill_timeout    = 600;
    puser_if_params->datajack_connect_timeout        = 15000;
    puser_if_params->datajack_pause_threshold        = 45;
    puser_if_params->datajack_ias_timer              = 10;
    *buffer                                          = pbuffer;

    print_user_if_params_table(puser_if_params);

}

static void generate_dlog_mt_end_data(mm_context_t *context, uint8_t **buffer, size_t *len) {
    *len    = 1;
    *buffer = (uint8_t *)calloc(1, *len);
    if (*buffer == NULL) {
        fprintf(stderr, "%s: Error allocating %zu bytes of memory\n", __func__, *len);
        mm_shutdown(context);
        exit(-ENOMEM);
    }

    *buffer[0] = DLOG_MT_END_DATA;
}

static int create_terminal_specific_directory(char *table_dir, char *terminal_id) {
    char dirname[268];
    int  status = 0;

    errno = 0;

    snprintf(dirname, sizeof(dirname), "%s/%s", table_dir, terminal_id);

#ifdef _WIN32
    status = _mkdir(dirname);
#else  /* ifdef _WIN32 */
    status = mkdir(dirname, 0755);
#endif /* ifdef _WIN32 */

    if ((status != 0) && (errno != EEXIST)) {
        fprintf(stderr, "%s: Failed to create directory: %s\n", __func__, dirname);
        return -ENOENT;
    }

    return status;
}

time_t mm_time(int test_mode, time_t *rawtime) {
    if (test_mode) {
        /* When in test mode, use a static time, so that results are consistent. */
        *rawtime = JAN12020;
    }
    else {
        time(rawtime);
    }

    return *rawtime;
}

static void mm_display_help(const char *name, FILE *stream) {
    /* "a:b:cd:e:f:hi:k:l:mn:p:qrst:uvw" */
    fprintf(stream,
        "usage: %s [-vhmq] [-f <filename>] [-i \"modem init string\"] [-l <logfile>] [-p <pcapfile>] [-a <access_code>] [-k <key_code>] [-n <ncc_number>] [-d <default_table_dir] [-t <term_table_dir>] [-u <port>]\n",
        name);
    fprintf(stream,
            "\t-a <access_code> - Craft 7-digit access code (default: CRASERV)\n" \
            "\t-b <baudrate> - Modem baud rate, in bps.  Defaults to 19200.\n" \
            "\t-c - Always download complete table set.\n" \
            "\t-d <default_table_dir> - default table directory.\n" \
            "\t-e <error_inject_type> - Inject error on SIGBRK.\n" \
            "\t-f <filename> modem device or file\n" \
            "\t-h this help.\n" \
            "\t-i \"modem init string\" - Modem initialization string.\n" \
            "\t-k <key_code> - Desk Terminal 10-digit key card code (default: 4012888888)\n" \
            "\t-l <logfile> - log bytes transmitted to and received from the terminal.  Useful for debugging.\n" \
            "\t-m use serial modem (specify device with -f)\n" \
            "\t-n <Primary NCC Number> [-n <Secondary NCC Number>] - specify primary and optionally secondary NCC number.\n" \
            "\t-p <pcapfile> - Save packets in a .pcap file.\n" \
            "\t-q - Don't display sign-on banner.\n" \
            "\t-r - Rating test mode: Amount charged determined by last 4 digits of dialed number.\n" \
            "\t-s - Download only minimum required tables to terminal.\n" \
            "\t-t <term_table_dir> - terminal-specific table directory.\n" \
            "\t-u <port> - Send packets as UDP to <port>.\n" \
            "\t-v verbose (multiple v's increase verbosity.\n" \
            "\t-w - don't monitor the modem for carrier loss.\n");
    return;
}
