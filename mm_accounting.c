/*
 * Accounting module for mm_manager.
 *
 * Stores call accounting and statistics data from the
 * Nortel Millennium payphone in an sqlite3 or MariaDB
 * database.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2022, Howard M. Harte
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./mm_manager.h"

#define TELCO_ID_REGION_CODE "\"%c%c\",\"%c%c%c\""

#ifdef MYSQL_DB
#define AUTO_INCREMENT  "AUTO_INCREMENT"
#define SQL_IGNORE      "IGNORE "
#else
#define AUTO_INCREMENT "AUTOINCREMENT"
#define SQL_IGNORE      ""
#endif /* MYSQL */


int mm_acct_save_TALARM(mm_context_t *context, dlog_mt_alarm_t *alarm) {
    char sql[512] = { 0 };
    char timestamp_str[20] = { 0 };
    char received_time_str[16] = { 0 };

    printf("\t\tAlarm: %s: Type: %d (0x%02x) - %s\n",
            timestamp_to_string(alarm->timestamp, timestamp_str, sizeof(timestamp_str)),
            alarm->alarm_id, alarm->alarm_id,
            alarm_id_to_string(alarm->alarm_id));

    snprintf(sql, sizeof(sql), "INSERT " SQL_IGNORE "INTO TALARM ( TERMINAL_ID, RECEIVED_DATE, RECEIVED_TIME, START_DATE, START_TIME, ALARM_ID, TELCO_ID, REGION_CODE,ALARM ) VALUES ( " \
                               " \"%s\",%s,%s,%d," TELCO_ID_REGION_CODE ",\"%s\");",
        context->terminal_id,
        received_time_to_db_string(received_time_str, sizeof(received_time_str)),
        timestamp_to_db_string(alarm->timestamp, timestamp_str, sizeof(timestamp_str)),
        alarm->alarm_id,
        context->telco_id[0], context->telco_id[1],
        context->region_code[0], context->region_code[1], context->region_code[2],
        alarm_id_to_string(alarm->alarm_id));

    return mm_sql_exec(context->database, sql);
}

int mm_acct_save_TAUTH(mm_context_t* context, dlog_mt_funf_card_auth_t* auth_request) {
    char sql[512] = { 0 };
    char phone_number_string[21] = { 0 };
    char card_number_string[25] = { 0 };
    char call_type_str[38] = { 0 };
    char received_time_str[16] = { 0 };

    phone_num_to_string(phone_number_string, sizeof(phone_number_string), auth_request->phone_number,
        sizeof(auth_request->phone_number));
    phone_num_to_string(card_number_string, sizeof(card_number_string), auth_request->card_number,
        sizeof(auth_request->card_number));
    call_type_to_string(auth_request->call_type, call_type_str, sizeof(call_type_str));

    printf("\t\tCard Auth request: Terminal: %s, Phone number: %s, seq=%d, card#: %s, exp: %02x/%02x, init: %02x/%02x, ctrlflag: 0x%02x carrier: %d, Call_type: %s, card_ref_num:0x%02x, unk:0x%04x, unk2:0x%04x\n",
        context->terminal_id,
        phone_number_string,
        auth_request->seq,
        card_number_string,
        auth_request->exp_mm,
        auth_request->exp_yy + 0x2000,
        auth_request->init_mm,
        auth_request->init_yy + 0x2000,
        auth_request->control_flag,
        auth_request->carrier_ref,
        call_type_str,
        auth_request->card_ref_num,
        auth_request->unknown,
        auth_request->unknown2);


    snprintf(sql, sizeof(sql), "INSERT " SQL_IGNORE "INTO TAUTH ( TERMINAL_ID, RECEIVED_DATE, RECEIVED_TIME,"
        "INTERNATIONAL_CALL_IND,"
        "CALLED_TELEPHONE_NO,"
        "CARRIER_XREF_NUMBER,"
        "CARD_NUMBER,"
        "SERVICE_CODE,"
        "CARD_EXPIRY_DATE,"
        "INITIAL_DATE,"
        "DISCRETIONARY,"
        "SPARE_FORMATTED,"
        "CARD_DATA,"
        "PIN,"
        "CALL_TYPE,"
        "CARD_REF_NO,"
        "SEQUENCE_NO,"
        "FOLLOW_ON_IND,"
        "TELCO_ID, REGION_CODE"
        ") VALUES ( " \
        " \"%s\",%s,%d,\"%s\",%d,\"%s\",%d,%04x%02x,%04x%02x,%d,%d,%d,%d,%d,%d,%d,%d," TELCO_ID_REGION_CODE ");",
        context->terminal_id,
        received_time_to_db_string(received_time_str, sizeof(received_time_str)),
        0,
        phone_number_string,
        auth_request->carrier_ref,
        card_number_string,
        0,
        auth_request->exp_yy + 0x2000,
        auth_request->exp_mm,
        auth_request->init_yy + 0x2000,
        auth_request->init_mm,
        auth_request->control_flag,
        0,
        0,
        0,
        auth_request->call_type,
        auth_request->card_ref_num,
        auth_request->seq,
        0,
        context->telco_id[0], context->telco_id[1],
        context->region_code[0], context->region_code[1], context->region_code[2]);

    return mm_sql_exec(context->database, sql);
}

int mm_acct_save_TCDR(mm_context_t *context, dlog_mt_call_details_t *cdr) {
    char sql[512] = { 0 };
    char timestamp_str[20] = { 0 };
    char received_time_str[16] = { 0 };
    char phone_number_string[21];
    char card_number_string[21];
    char call_type_str[38];

    printf(
        "\t\tCDR: %s, Duration: %02d:%02d:%02d %s, DN: %s, Card#: %s, Collected: $%3.2f, Requested: $%3.2f, carrier code=%d, rate_type=%d, Seq: %04d\n",
        timestamp_to_string(cdr->start_timestamp, timestamp_str, sizeof(timestamp_str)),
        cdr->call_duration[0],
        cdr->call_duration[1],
        cdr->call_duration[2],
        call_type_to_string(cdr->call_type, call_type_str, sizeof(call_type_str)),
        phone_num_to_string(phone_number_string, sizeof(phone_number_string), cdr->called_num,
                            sizeof(cdr->called_num)),
        phone_num_to_string(card_number_string,  sizeof(card_number_string),  cdr->card_num,
                            sizeof(cdr->card_num)),
        (float)cdr->call_cost[0] / 100,
        (float)cdr->call_cost[1] / 100,
        cdr->carrier_code,
        cdr->rate_type,
        cdr->seq);

    if (context->debuglevel > 2) {
        printf("\t\t\tDLOG_MT_CALL_DETAILS Pad:");
        dump_hex(cdr->pad, sizeof(cdr->pad));
    }

    snprintf(sql, sizeof(sql), "INSERT " SQL_IGNORE "INTO TCDR ( TERMINAL_ID,RECEIVED_DATE,RECEIVED_TIME,SEQ,START_DATE,START_TIME,CALL_DURATION,CD_CALL_TYPE,CD_CALL_TYPE_STR,DIALED_NUM,CARD,REQUESTED,COLLECTED,CARRIER,RATE,TELCO_ID,REGION_CODE) VALUES ( " \
                               " \"%s\",%s,%d,%s,%d,%d,\"%s\",\"%s\",\"%s\",\"%3.2f\",\"%3.2f\",%d,%d," TELCO_ID_REGION_CODE ");",
        context->terminal_id,
        received_time_to_db_string(received_time_str, sizeof(received_time_str)),
        cdr->seq,
        timestamp_to_db_string(cdr->start_timestamp, timestamp_str, sizeof(timestamp_str)),
        cdr->call_duration[0] * 3600 +
        cdr->call_duration[1] * 60 +
        cdr->call_duration[2],
        cdr->call_type,
        call_type_to_string(cdr->call_type, call_type_str, sizeof(call_type_str)),
        phone_num_to_string(phone_number_string, sizeof(phone_number_string), cdr->called_num,
                            sizeof(cdr->called_num)),
        phone_num_to_string(card_number_string,  sizeof(card_number_string),  cdr->card_num,
                            sizeof(cdr->card_num)),
        (float)cdr->call_cost[1] / 100,
        (float)cdr->call_cost[0] / 100,
        cdr->carrier_code,
        cdr->rate_type,
        context->telco_id[0], context->telco_id[1],
        context->region_code[0], context->region_code[1], context->region_code[2]);

    return mm_sql_exec(context->database, sql);
}

int mm_acct_save_TCALLST(mm_context_t* context, dlog_mt_summary_call_stats_t* summary_call_stats) {
    char sql[1024] = { 0 };
    char received_time_str[16] = { 0 };
    char timestamp_str[20] = { 0 };
    char timestamp2_str[20] = { 0 };
    char timestamp3_str[20] = { 0 };
    char timestamp4_str[20] = { 0 };

    printf("\t\t\tSummary Call Statistics: From: %s, to: %s:\n",
        timestamp_to_string(summary_call_stats->start_timestamp, timestamp_str, sizeof(timestamp_str)),
        timestamp_to_string(summary_call_stats->end_timestamp, timestamp2_str, sizeof(timestamp2_str)));

    for (int j = 0; j < 16; j++) {
        printf("\t\t\t\t%s: %5d\n", TCALSTE_stats_to_str(j), summary_call_stats->stats[j]);
    }
    printf("\n\t\t\t\tRep Dialer Peg Counts:\t");

    for (int j = 0; j < 10; j++) {
        if (j == 5) {
            printf("\n\t\t\t\t\t\t\t");
        }
        printf("%d, ", summary_call_stats->rep_dialer_peg_count[j]);
    }

    seconds_to_ddhhmmss_string(timestamp3_str, sizeof(timestamp3_str), summary_call_stats->total_call_duration);
    printf("\n\t\t\t\tTotal Call duration: %s (%us)\n",
        timestamp3_str, summary_call_stats->total_call_duration);

    seconds_to_ddhhmmss_string(timestamp4_str, sizeof(timestamp4_str), summary_call_stats->total_time_off_hook);
    printf("\t\t\t\tTotal Off-hook duration: %s (%us)\n", timestamp4_str, summary_call_stats->total_time_off_hook);

    printf("\t\t\t\tFree Feature B Call Count: %d\n", summary_call_stats->free_featb_call_count);
    printf("\t\t\t\tCompleted 1-800 billable Count: %d\n", summary_call_stats->completed_1800_billable_count);
    printf("\t\t\t\tDatajack calls attempted: %d\n", summary_call_stats->datajack_calls_attempt_count);
    printf("\t\t\t\tDatajack calls completed: %d\n", summary_call_stats->datajack_calls_complete_count);

    snprintf(sql, sizeof(sql), "INSERT " SQL_IGNORE "INTO TCALLST("
        "TERMINAL_ID,"
        "RECEIVED_DATE, RECEIVED_TIME,"
        "SUMMARY_PERIOD_START_DATE,SUMMARY_PERIOD_START_TIME,"
        "SUMMARY_PERIOD_STOP_DATE,SUMMARY_PERIOD_STOP_TIME,"
        "TOTAL_CARD_CALL_CNT,"
        "FREE_CALL_CNT,"
        "INCOMING_CALL_CNT,"
        "UNANSWERED_CALL_CNT,"
        "ABANDONED_CALL_CNT,"
        "LOCAL_CARD_CALL_CNT,"
        "TOLL_CARD_CALL_CNT,"
        "OPERATOR_CALL_CNT,"
        "ZERO_PLUS_CALL_CNT,"
        "FOLLOW_ON_CALL_CNT,"
        "TOTAL_COIN_CALL_CNT,"
        "LOCAL_COIN_CALL_CNT,"
        "TOLL_COIN_CALL_CNT,"
        "FAIL_TO_POTS_COIN_CNT,"
        "INTER_LATA_TOLL_CARD_CALL_CNT,"
        "INTER_LATA_TOLL_COIN_CALL_CNT,"
        "REP_DIALER_PEG_CNT1,"
        "REP_DIALER_PEG_CNT2,"
        "REP_DIALER_PEG_CNT3,"
        "REP_DIALER_PEG_CNT4,"
        "REP_DIALER_PEG_CNT5,"
        "REP_DIALER_PEG_CNT6,"
        "REP_DIALER_PEG_CNT7,"
        "REP_DIALER_PEG_CNT8,"
        "REP_DIALER_PEG_CNT9,"
        "REP_DIALER_PEG_CNT10,"
        "TOTAL_CALL_DURATION,"
        "TOTAL_TIME_OFF_HOOK,"
        "TELCO_ID, REGION_CODE"
        ") VALUES ( "
        "\"%s\",%s,"
        "%s, %s,"
        "%d,%d,%d,%d,%d,%d,%d,%d,"
        "%d,%d,%d,%d,%d,%d,%d,%d,"
        "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"    /* Rep dialer peg counts */
        "\"%s\", \"%s\"," TELCO_ID_REGION_CODE ");",
        context->terminal_id,
        received_time_to_db_string(received_time_str, sizeof(received_time_str)),
        timestamp_to_db_string(summary_call_stats->start_timestamp, timestamp_str, sizeof(timestamp_str)),
        timestamp_to_db_string(summary_call_stats->end_timestamp, timestamp2_str, sizeof(timestamp2_str)),
        summary_call_stats->stats[0], summary_call_stats->stats[1], summary_call_stats->stats[2], summary_call_stats->stats[3],
        summary_call_stats->stats[4], summary_call_stats->stats[5], summary_call_stats->stats[6], summary_call_stats->stats[7],
        summary_call_stats->stats[8], summary_call_stats->stats[9], summary_call_stats->stats[10], summary_call_stats->stats[11],
        summary_call_stats->stats[12], summary_call_stats->stats[13], summary_call_stats->stats[14], summary_call_stats->stats[15],
        summary_call_stats->rep_dialer_peg_count[0], summary_call_stats->rep_dialer_peg_count[1],
        summary_call_stats->rep_dialer_peg_count[2], summary_call_stats->rep_dialer_peg_count[3],
        summary_call_stats->rep_dialer_peg_count[4], summary_call_stats->rep_dialer_peg_count[5],
        summary_call_stats->rep_dialer_peg_count[6], summary_call_stats->rep_dialer_peg_count[7],
        summary_call_stats->rep_dialer_peg_count[8], summary_call_stats->rep_dialer_peg_count[9],
        timestamp3_str,
        timestamp4_str,
        context->telco_id[0], context->telco_id[1],
        context->region_code[0], context->region_code[1], context->region_code[2]);

    return mm_sql_exec(context->database, sql);
}

int mm_acct_load_TCASHST(mm_context_t* context, cashbox_status_univ_t* cashbox_status) {
    char timestamp_str[20] = { 0 };

    /* Retrieve cash box status for the current terminal. */
    mm_sql_load_TCASHST(context->database, context->terminal_id, cashbox_status);

    printf("Load Cashbox status: %s Total: $%3.2f (%3d%% full): CA N:%d D:%d Q:%d $:%d - US N:%d D:%d Q:%d $:%d\n",
        timestamp_to_string(cashbox_status->timestamp, timestamp_str, sizeof(timestamp_str)),
        (float)cashbox_status->currency_value / 100.0,
        cashbox_status->percent_full,
        cashbox_status->coin_count[COIN_COUNT_CA_NICKELS],
        cashbox_status->coin_count[COIN_COUNT_CA_DIMES],
        cashbox_status->coin_count[COIN_COUNT_CA_QUARTERS],
        cashbox_status->coin_count[COIN_COUNT_CA_DOLLARS],
        cashbox_status->coin_count[COIN_COUNT_US_NICKELS],
        cashbox_status->coin_count[COIN_COUNT_US_DIMES],
        cashbox_status->coin_count[COIN_COUNT_US_QUARTERS],
        cashbox_status->coin_count[COIN_COUNT_US_DOLLARS]);

    return 0;
}

int mm_acct_save_TCASHST(mm_context_t* context, cashbox_status_univ_t* cashbox_status) {
    char sql[512] = { 0 };
    char received_time_str[16] = { 0 };
    char timestamp_str[20];

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

    snprintf(sql, sizeof(sql), "REPLACE INTO TCASHST ( "
        "TERMINAL_ID,RECEIVED_DATE,RECEIVED_TIME,START_DATE,START_TIME, CASH_BOX_STATUS, PERCENT_FULL, CURRENCY_VALUE,"
        "NUMBER_OF_CDN_NICKELS, NUMBER_OF_CDN_DIMES, NUMBER_OF_CDN_QUARTERS, NUMBER_OF_CDN_DOLLARS,"
        "NUMBER_OF_US_NICKELS,  NUMBER_OF_US_DIMES,  NUMBER_OF_US_QUARTERS,  NUMBER_OF_US_DOLLARS,"
        "TELCO_ID, REGION_CODE"
        ") VALUES ( "
        "\"%s\",%s,%s,%d,%d,\"%3.2f\", "
        "%d, %d, %d, %d, %d, %d, %d, %d, " TELCO_ID_REGION_CODE ");",
        context->terminal_id,
        received_time_to_db_string(received_time_str, sizeof(received_time_str)),
        timestamp_to_db_string(cashbox_status->timestamp, timestamp_str, sizeof(timestamp_str)),
        cashbox_status->percent_full,
        cashbox_status->status,
        (float)cashbox_status->currency_value / 100,
        cashbox_status->coin_count[COIN_COUNT_CA_NICKELS],
        cashbox_status->coin_count[COIN_COUNT_CA_DIMES],
        cashbox_status->coin_count[COIN_COUNT_CA_QUARTERS],
        cashbox_status->coin_count[COIN_COUNT_CA_DOLLARS],
        cashbox_status->coin_count[COIN_COUNT_US_NICKELS],
        cashbox_status->coin_count[COIN_COUNT_US_DIMES],
        cashbox_status->coin_count[COIN_COUNT_US_QUARTERS],
        cashbox_status->coin_count[COIN_COUNT_US_DOLLARS],
        context->telco_id[0], context->telco_id[1],
        context->region_code[0], context->region_code[1], context->region_code[2]);

    return mm_sql_exec(context->database, sql);
}

int mm_acct_save_TCOLLST(mm_context_t* context, dlog_mt_cash_box_collection_t* cash_box_collection) {
    char sql[512] = { 0 };
    char received_time_str[16] = { 0 };
    char timestamp_str[20];

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

    if (context->debuglevel > 2) {
        printf("\t\t\tDLOG_MT_CASH_BOX_COLLECTION Pad:");
        dump_hex(cash_box_collection->pad, sizeof(cash_box_collection->pad));
        printf("\t\t\tDLOG_MT_CASH_BOX_COLLECTION Pad2:");
        dump_hex(cash_box_collection->pad2, sizeof(cash_box_collection->pad2));
        printf("\t\t\tDLOG_MT_CASH_BOX_COLLECTION Pad3:");
        dump_hex(cash_box_collection->pad3, sizeof(cash_box_collection->pad3));
        printf("\t\t\tDLOG_MT_CASH_BOX_COLLECTION Spare:");
        dump_hex(cash_box_collection->spare, sizeof(cash_box_collection->spare));
    }

    snprintf(sql, sizeof(sql), "INSERT " SQL_IGNORE "INTO TCOLLST ( "
        "TERMINAL_ID,RECEIVED_DATE,RECEIVED_TIME,COLLECTION_DATE,COLLECTION_TIME,"
        "CASH_BOX_STATUS, PERCENT_FULL, CURRENCY_VALUE,"
        "NUMBER_OF_CDN_NICKELS, NUMBER_OF_CDN_DIMES, NUMBER_OF_CDN_QUARTERS, NUMBER_OF_CDN_DOLLARS,"
        "NUMBER_OF_US_NICKELS,  NUMBER_OF_US_DIMES,  NUMBER_OF_US_QUARTERS,  NUMBER_OF_US_DOLLARS, "
        "TELCO_ID, REGION_CODE ) VALUES ( "
        "\"%s\",%s,%s,%d,%d,\"%3.2f\", "
        "%d, %d, %d, %d, %d, %d, %d, %d," TELCO_ID_REGION_CODE ")",
        context->terminal_id,
        received_time_to_db_string(received_time_str, sizeof(received_time_str)),
        timestamp_to_db_string(cash_box_collection->timestamp, timestamp_str, sizeof(timestamp_str)),
        cash_box_collection->percent_full,
        cash_box_collection->status,
        (float)cash_box_collection->currency_value / 100,
        cash_box_collection->coin_count[COIN_COUNT_CA_NICKELS],
        cash_box_collection->coin_count[COIN_COUNT_CA_DIMES],
        cash_box_collection->coin_count[COIN_COUNT_CA_QUARTERS],
        cash_box_collection->coin_count[COIN_COUNT_CA_DOLLARS],
        cash_box_collection->coin_count[COIN_COUNT_US_NICKELS],
        cash_box_collection->coin_count[COIN_COUNT_US_DIMES],
        cash_box_collection->coin_count[COIN_COUNT_US_QUARTERS],
        cash_box_collection->coin_count[COIN_COUNT_US_DOLLARS],
        context->telco_id[0], context->telco_id[1],
        context->region_code[0], context->region_code[1], context->region_code[2]);

    return mm_sql_exec(context->database, sql);
}

int mm_acct_save_TOPCODE(mm_context_t *context, dlog_mt_maint_req_t *maint) {
    char sql[256] = { 0 };
    char received_time_str[16] = { 0 };

    printf("\t\tMaintenance Type: %d (0x%03x) Access PIN: %02x%02x%01x\n",
        maint->type, maint->type,
        maint->access_pin[0], maint->access_pin[1], (maint->access_pin[2] & 0xF0) >> 4);

    snprintf(sql, sizeof(sql), "INSERT INTO TOPCODE ( TERMINAL_ID,RECEIVED_DATE,RECEIVED_TIME,OP_CODE,PIN,TELCO_ID,REGION_CODE ) VALUES ( " \
                               " \"%s\",%s,%d,\" %02x%02x%01x\", " TELCO_ID_REGION_CODE ")",
        context->terminal_id,
        received_time_to_db_string(received_time_str, sizeof(received_time_str)),
        maint->type,
        maint->access_pin[0], maint->access_pin[1], (maint->access_pin[2] & 0xF0) >> 4,
        context->telco_id[0], context->telco_id[1],
        context->region_code[0], context->region_code[1], context->region_code[2]);

    return mm_sql_exec(context->database, sql);
}

int mm_acct_save_TPERFST(mm_context_t* context, dlog_mt_perf_stats_record_t* perf_stats) {
    char sql[1536] = { 0 };
    char received_time_str[16] = { 0 };
    char timestamp_str[20];
    char timestamp2_str[20];

    snprintf(sql, sizeof(sql), "INSERT " SQL_IGNORE "INTO TPERFST("
        "TERMINAL_ID,"
        "RECEIVED_DATE, RECEIVED_TIME,"
        "SUMMARY_PERIOD_START_DATE,"
        "SUMMARY_PERIOD_START_TIME,"
        "SUMMARY_PERIOD_STOP_DATE,"
        "SUMMARY_PERIOD_STOP_TIME,"
        "CALL_ATTEMPTS_CNT,"
        "BUSY_SIGNAL_CNT,"
        "CALL_CLEARED_NO_DATA,"
        "NO_CARRIER_DETECT_CNT,"
        "CO_ACCESS_TO_DIAL_CNT1,"
        "CO_ACCESS_TO_DIAL_CNT2,"
        "CO_ACCESS_TO_DIAL_CNT3,"
        "CO_ACCESS_TO_DIAL_CNT4,"
        "CO_ACCESS_TO_DIAL_CNT5,"
        "CO_ACCESS_TO_DIAL_CNT6,"
        "CO_ACCESS_TO_DIAL_CNT7,"
        "DIAL_TO_CARRIER_CNT1,"
        "DIAL_TO_CARRIER_CNT2,"
        "DIAL_TO_CARRIER_CNT3,"
        "DIAL_TO_CARRIER_CNT4,"
        "DIAL_TO_CARRIER_CNT5,"
        "DIAL_TO_CARRIER_CNT6,"
        "DIAL_TO_CARRIER_CNT7,"
        "CARRIER_TO_1ST_PACKET_CNT1,"
        "CARRIER_TO_1ST_PACKET_CNT2,"
        "CARRIER_TO_1ST_PACKET_CNT3,"
        "CARRIER_TO_1ST_PACKET_CNT4,"
        "CARRIER_TO_1ST_PACKET_CNT5,"
        "CARRIER_TO_1ST_PACKET_CNT6,"
        "CARRIER_TO_1ST_PACKET_CNT7,"
        "USER_WAIT_TO_EXPECT_INFO_CNT1,"
        "USER_WAIT_TO_EXPECT_INFO_CNT2,"
        "USER_WAIT_TO_EXPECT_INFO_CNT3,"
        "USER_WAIT_TO_EXPECT_INFO_CNT4,"
        "USER_WAIT_TO_EXPECT_INFO_CNT5,"
        "USER_WAIT_TO_EXPECT_INFO_CNT6,"
        "USER_WAIT_TO_EXPECT_INFO_CNT7,"
        "TOTAL_DIALOGS_FAILED,"
        "NO_PACKET_RCVD_ERRORS,"
        "NO_PACKET_RETRIES_RCVD,"
        "INACTIVITY_COUNT,"
        "RETRY_LIMIT_OUT_OF_SERVICE,"
        "CARD_AUTH_TIMEOUTS,"
        "RATE_REQUEST_TIMEOUTS,"
        "NO_DIAL_TONE,"
        "SPARE1,"
        "SPARE2,"
        "SPARE3,"
        "TELCO_ID, REGION_CODE"
        ") VALUES ( "
        "\"%s\",%s,"
        "%s, %s,"
        "%d,%d,%d,%d,%d,%d,%d,%d,"
        "%d,%d,%d,%d,%d,%d,%d,%d,"
        "%d,%d,%d,%d,%d,%d,%d,%d,"
        "%d,%d,%d,%d,%d,%d,%d,%d,"
        "%d,%d,%d,%d,%d,%d,%d,%d,"
        "%d,%d,%d," TELCO_ID_REGION_CODE ");",
        context->terminal_id,
        received_time_to_db_string(received_time_str, sizeof(received_time_str)),
        timestamp_to_db_string(perf_stats->timestamp, timestamp_str, sizeof(timestamp_str)),
        timestamp_to_db_string(perf_stats->timestamp2, timestamp2_str, sizeof(timestamp2_str)),
        perf_stats->stats[0], perf_stats->stats[1], perf_stats->stats[2], perf_stats->stats[3],
        perf_stats->stats[4], perf_stats->stats[5], perf_stats->stats[6], perf_stats->stats[7],
        perf_stats->stats[8], perf_stats->stats[9], perf_stats->stats[10], perf_stats->stats[11],
        perf_stats->stats[12], perf_stats->stats[13], perf_stats->stats[14], perf_stats->stats[15],
        perf_stats->stats[16], perf_stats->stats[17], perf_stats->stats[18], perf_stats->stats[19],
        perf_stats->stats[20], perf_stats->stats[21], perf_stats->stats[22], perf_stats->stats[23],
        perf_stats->stats[24], perf_stats->stats[25], perf_stats->stats[26], perf_stats->stats[27],
        perf_stats->stats[28], perf_stats->stats[29], perf_stats->stats[30], perf_stats->stats[31],
        perf_stats->stats[32], perf_stats->stats[33], perf_stats->stats[34], perf_stats->stats[35],
        perf_stats->stats[36], perf_stats->stats[37], perf_stats->stats[38], perf_stats->stats[39],
        perf_stats->stats[40], perf_stats->stats[41], perf_stats->stats[42],
        context->telco_id[0], context->telco_id[1],
        context->region_code[0], context->region_code[1], context->region_code[2]);

    mm_sql_exec(context->database, sql);

    printf("\t\tPerformance Statistics Record: From: %s, to: %s:\n",
        timestamp_to_string(perf_stats->timestamp, timestamp_str, sizeof(timestamp_str)),
        timestamp_to_string(perf_stats->timestamp2, timestamp2_str, sizeof(timestamp2_str)));

    for (size_t i = 0; i < ((sizeof(perf_stats->stats) / sizeof(uint16_t))); i++) {
        if (perf_stats->stats[i] > 0) printf("[%2zu] %27s: %5d\n", i, TPERFST_stats_to_str((uint8_t)i), perf_stats->stats[i]);
    }

    return 0;
}

int mm_acct_save_TSTATUS(mm_context_t* context, dlog_mt_term_status_t* dlog_mt_term_status) {
    char sql[1536] = { 0 };
    char received_time_str[16] = { 0 };
    uint8_t  serial_number[11] = { 0 };
    uint64_t term_status_word;
    uint64_t last_status_word = 0LL;
    int i;

    for (i = 0; i < 5; i++) {
        serial_number[i * 2] = ((dlog_mt_term_status->serialnum[i] & 0xf0) >> 4) + '0';
        serial_number[i * 2 + 1] = (dlog_mt_term_status->serialnum[i] & 0x0f) + '0';
    }

    serial_number[10] = '\0';

    term_status_word  = dlog_mt_term_status->status[0];
    term_status_word |= (uint64_t)(dlog_mt_term_status->status[1]) << 8;
    term_status_word |= (uint64_t)(dlog_mt_term_status->status[2]) << 16;
    term_status_word |= (uint64_t)(dlog_mt_term_status->status[3]) << 24;
    term_status_word |= (uint64_t)(dlog_mt_term_status->status[4]) << 32;

    printf("\t\tTerminal serial number %s, Terminal Status Word: 0x%010" PRIx64 "\n",
        serial_number, term_status_word);

    /* Retrieve the most recent terminal status, and update only if changed. */
    snprintf(sql, sizeof(sql), "SELECT STATUS_WORD from TSTATUS where(TERMINAL_ID = %s) ORDER BY ID DESC LIMIT 1",
        context->terminal_id);

    last_status_word = mm_sql_read_uint64(context->database, sql);

    if (term_status_word != last_status_word) {
        snprintf(sql, sizeof(sql), "INSERT INTO TSTATUS ( "
            "TERMINAL_ID,"
            "RECEIVED_DATE, RECEIVED_TIME,"
            "SERIAL_NO,"
            "STATUS_WORD,"
            "HANDSET_DISCONT_IND,"
            "TELEPHONY_STATUS_IND,"
            "EPM_SAM_NOT_RESPONDING,"
            "EPM_SAM_LOCKED_OUT,"
            "EPM_SAM_EXPIRED,"
            "EPM_SAM_REACHING_TRANS_LIMIT,"
            "UNABLE_REACH_PRIM_COL_SYS,"
            "TELEPHONY_STATUS_BIT_7,"
            "POWER_FAIL_IND,"
            "DISPLAY_RESPONSE_IND,"
            "VOICE_SYNTHESIS_RESPONSE_IND,"
            "UNABLE_REACH_SECOND_COL_SYS,"
            "CARD_READER_BLOCKED_ALARM,"
            "MANDATORY_TABLE_ALARM,"
            "DATAJACK_PORT_BLOCKED,"
            "CTRL_HW_STATUS_BIT_7,"
            "CDR_CHECKSUM_ERR_IND,"
            "STATISTICS_CHECKSUM_ERR_IND,"
            "TERMINAL_TBL_CHECKSUM_ERR_IND,"
            "OTHER_DATA_CHECKSUM_ERR_IND,"
            "CDR_LIST_FULL_ERR_IND,"
            "BAD_EEPROM_ERR_IND,"
            "MEMORY_LOST_ERROR_IND,"
            "MEMORY_BAD_ERR_IND,"
            "ACCESS_COVER_IND,"
            "KEY_MATRIX_MALFUNC_IND,"
            "SET_REMOVAL_IND,"
            "THRESHOLD_MET_EXCEEDED_IND,"
            "CASH_BOX_COVER_OPEN_IND,"
            "CASH_BOX_REMOVED_IND,"
            "COIN_BOX_FULL_IND,"
            "COIN_JAM_COIN_CHUTE_IND,"
            "ESCROW_JAM_IND,"
            "VAL_HARDWARE_FAIL_IND,"
            "CO_LINE_CHECK_FAIL_IND,"
            "DIALOG_FAILURE_IND,"
            "CASH_BOX_ELECTRONIC_LOCK_IND,"
            "DIALOG_FAILURE_WITH_COL_SYS,"
            "CODE_SERVE_CONNECTION_FAILURE,"
            "CODE_SERVER_ABORTED,"
            "TELCO_ID, REGION_CODE"
            ") VALUES ( "
            "\"%s\",%s,\"%s\",%" PRIu64 ","
            "%d,%d,%d,%d,%d,%d,%d,%d,"
            "%d,%d,%d,%d,%d,%d,%d,%d,"
            "%d,%d,%d,%d,%d,%d,%d,%d,"
            "%d,%d,%d,%d,%d,%d,%d,%d,"
            "%d,%d,%d,%d,%d,%d,%d,%d,"
            TELCO_ID_REGION_CODE ")",
            context->terminal_id,
            received_time_to_db_string(received_time_str, sizeof(received_time_str)),
            serial_number,
            term_status_word,
            (term_status_word & TSTATUS_HANDSET_DISCONT_IND) ? 1 : 0,
            (term_status_word & TSTATUS_TELEPHONY_STATUS_IND) ? 1 : 0,
            (term_status_word & TSTATUS_EPM_SAM_NOT_RESPONDING) ? 1 : 0,
            (term_status_word & TSTATUS_EPM_SAM_LOCKED_OUT) ? 1 : 0,
            (term_status_word & TSTATUS_EPM_SAM_EXPIRED) ? 1 : 0,
            (term_status_word & TSTATUS_EPM_SAM_REACHING_TRANS_LIMIT) ? 1 : 0,
            (term_status_word & TSTATUS_UNABLE_REACH_PRIM_COL_SYS) ? 1 : 0,
            (term_status_word & TSTATUS_TELEPHONY_STATUS_BIT_7) ? 1 : 0,
            (term_status_word & TSTATUS_POWER_FAIL_IND) ? 1 : 0,
            (term_status_word & TSTATUS_DISPLAY_RESPONSE_IND) ? 1 : 0,
            (term_status_word & TSTATUS_VOICE_SYNTHESIS_RESPONSE_IND) ? 1 : 0,
            (term_status_word & TSTATUS_UNABLE_REACH_SECOND_COL_SYS) ? 1 : 0,
            (term_status_word & TSTATUS_CARD_READER_BLOCKED_ALARM) ? 1 : 0,
            (term_status_word & TSTATUS_MANDATORY_TABLE_ALARM) ? 1 : 0,
            (term_status_word & TSTATUS_DATAJACK_PORT_BLOCKED) ? 1 : 0,
            (term_status_word & TSTATUS_CTRL_HW_STATUS_BIT_7) ? 1 : 0,
            (term_status_word & TSTATUS_CDR_CHECKSUM_ERR_IND) ? 1 : 0,
            (term_status_word & TSTATUS_STATISTICS_CHECKSUM_ERR_IND) ? 1 : 0,
            (term_status_word & TSTATUS_TERMINAL_TBL_CHECKSUM_ERR_IND) ? 1 : 0,
            (term_status_word & TSTATUS_OTHER_DATA_CHECKSUM_ERR_IND) ? 1 : 0,
            (term_status_word & TSTATUS_CDR_LIST_FULL_ERR_IND) ? 1 : 0,
            (term_status_word & TSTATUS_BAD_EEPROM_ERR_IND) ? 1 : 0,
            (term_status_word & TSTATUS_MEMORY_LOST_ERROR_IND) ? 1 : 0,
            (term_status_word & TSTATUS_MEMORY_BAD_ERR_IND) ? 1 : 0,
            (term_status_word & TSTATUS_ACCESS_COVER_IND) ? 1 : 0,
            (term_status_word & TSTATUS_KEY_MATRIX_MALFUNC_IND) ? 1 : 0,
            (term_status_word & TSTATUS_SET_REMOVAL_IND) ? 1 : 0,
            (term_status_word & TSTATUS_THRESHOLD_MET_EXCEEDED_IND) ? 1 : 0,
            (term_status_word & TSTATUS_CASH_BOX_COVER_OPEN_IND) ? 1 : 0,
            (term_status_word & TSTATUS_CASH_BOX_REMOVED_IND) ? 1 : 0,
            (term_status_word & TSTATUS_COIN_BOX_FULL_IND) ? 1 : 0,
            (term_status_word & TSTATUS_COIN_JAM_COIN_CHUTE_IND) ? 1 : 0,
            (term_status_word & TSTATUS_ESCROW_JAM_IND) ? 1 : 0,
            (term_status_word & TSTATUS_VAL_HARDWARE_FAIL_IND) ? 1 : 0,
            (term_status_word & TSTATUS_CO_LINE_CHECK_FAIL_IND) ? 1 : 0,
            (term_status_word & TSTATUS_DIALOG_FAILURE_IND) ? 1 : 0,
            (term_status_word & TSTATUS_CASH_BOX_ELECTRONIC_LOCK_IND) ? 1 : 0,
            (term_status_word & TSTATUS_DIALOG_FAILURE_WITH_COL_SYS) ? 1 : 0,
            (term_status_word & TSTATUS_CODE_SERVE_CONNECTION_FAILURE) ? 1 : 0,
            (term_status_word & TSTATUS_CODE_SERVER_ABORTED) ? 1 : 0,
            context->telco_id[0], context->telco_id[1],
            context->region_code[0], context->region_code[1], context->region_code[2]);

        if (mm_sql_exec(context->database, sql) != 0) {
            fprintf(stderr, "%s: Failed to save TSTATUS.", __func__);
            return 1;
        }
    }
    /* Iterate over all the terminal status bits and display a message for any flags set. */
    for (i = 0; term_status_word != 0; i++) {
        if (term_status_word & 1) {
            printf("\t\t\tTerminal Status: %s\n", alarm_id_to_string(i));
        }
        term_status_word >>= 1;
    }

    return 0;
}

int mm_acct_save_TSWVERS(mm_context_t* context, dlog_mt_sw_version_t* dlog_mt_sw_version) {
    char sql[512] = { 0 };
    char received_time_str[16] = { 0 };

    char control_rom_edition[sizeof(dlog_mt_sw_version->control_rom_edition) + 1] = { 0 };
    char control_version[sizeof(dlog_mt_sw_version->control_version) + 1] = { 0 };
    char telephony_rom_edition[sizeof(dlog_mt_sw_version->telephony_rom_edition) + 1] = { 0 };
    char telephony_version[sizeof(dlog_mt_sw_version->telephony_version) + 1] = { 0 };
    char validator_sw_ver[sizeof(dlog_mt_sw_version->validator_sw_ver) + 1] = { 0 };
    char validator_hw_ver[sizeof(dlog_mt_sw_version->validator_sw_ver) + 1] = { 0 };
    memcpy(control_rom_edition, dlog_mt_sw_version->control_rom_edition,
        sizeof(dlog_mt_sw_version->control_rom_edition));
    memcpy(control_version, dlog_mt_sw_version->control_version,
        sizeof(dlog_mt_sw_version->control_version));
    memcpy(telephony_rom_edition, dlog_mt_sw_version->telephony_rom_edition,
        sizeof(dlog_mt_sw_version->telephony_rom_edition));
    memcpy(telephony_version, dlog_mt_sw_version->telephony_version,
        sizeof(dlog_mt_sw_version->telephony_version));
    memcpy(validator_sw_ver, dlog_mt_sw_version->validator_sw_ver,
        sizeof(dlog_mt_sw_version->validator_sw_ver));
    memcpy(validator_hw_ver, dlog_mt_sw_version->validator_hw_ver,
        sizeof(dlog_mt_sw_version->validator_hw_ver));

    context->terminal_type = mm_config_get_term_type_from_control_rom_edition(context->database, control_rom_edition);

    if (context->terminal_type == MTR_UNKNOWN) {
        printf("Error: Unknown control ROM edition %s\n", control_rom_edition);
    }

    printf("\t\t\t             Terminal Type: %02d (0x%02x)\n",
        context->terminal_type,
        context->terminal_type);
    printf("\t\t\t     Feature Terminal Type: %s %02d (0x%02x)\n",
        feature_term_type_to_str(dlog_mt_sw_version->term_type),
        dlog_mt_sw_version->term_type,
        dlog_mt_sw_version->term_type);
    printf("\t\t\t       Control ROM Edition: %s\n", control_rom_edition);
    printf("\t\t\t           Control Version: %s\n", control_version);
    printf("\t\t\t     Telephony ROM Edition: %s\n", telephony_rom_edition);
    printf("\t\t\t         Telephony Version: %s\n", telephony_version);
    printf("\t\t\tValidator Hardware Version: %s\n", validator_hw_ver);
    printf("\t\t\tValidator Software Version: %s\n", validator_sw_ver);

    snprintf(sql, sizeof(sql), "INSERT " SQL_IGNORE "INTO TSWVERS ( "
        "TERMINAL_ID,EFFECTIVE_DATE,EFFECTIVE_TIME,"
        "CONTROL_ROM_EDITION,CONTROL_VERSION_NO,TELEPHONY_ROM_EDITION,TELEPHONY_VERSION_NO,"
        "FEATURE_TERMINAL_TYPE,TERMINAL_TYPE,VALIDATOR_SOFTWARE_VERS,VALIDATOR_HARDWARE_VERS,"
        "TELCO_ID, REGION_CODE"
        " ) VALUES ( "
        "\"%s\",%s,\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,\"%s\",\"%s\"," TELCO_ID_REGION_CODE ")",
        context->terminal_id,
        received_time_to_db_string(received_time_str, sizeof(received_time_str)),
        control_rom_edition,
        control_version,
        telephony_rom_edition,
        telephony_version,
        0,
        dlog_mt_sw_version->term_type,
        validator_sw_ver,
        validator_hw_ver,
        context->telco_id[0], context->telco_id[1],
        context->region_code[0], context->region_code[1], context->region_code[2]);

    return mm_sql_exec(context->database, sql);
}

int mm_acct_create_tables(void *db) {
    int rc;

    rc = mm_sql_exec(db, "CREATE TABLE IF NOT EXISTS TALARM ( "
        "ID INTEGER NOT NULL PRIMARY KEY " AUTO_INCREMENT ","
        "TERMINAL_ID VARCHAR(10) NOT NULL,"
        "RECEIVED_DATE VARCHAR(8) NOT NULL,"
        "RECEIVED_TIME VARCHAR(6) NOT NULL,"
        "START_DATE VARCHAR(8) NOT NULL,"
        "START_TIME VARCHAR(6) NOT NULL,"
        "ALARM_ID INTEGER,"
        "TELCO_ID VARCHAR(2) DEFAULT 0, REGION_CODE VARCHAR(3) DEFAULT \"USA\", ARCHIVE_IND BOOLEAN DEFAULT 0,"
        "ALARM TEXT,"
        "UNIQUE(TERMINAL_ID,START_DATE,START_TIME,ALARM_ID));");

    if (rc != 0) {
        fprintf(stderr, "%s: Failed to create table TALARM.\n", __func__);
        return -1;
    }

    rc = mm_sql_exec(db, "CREATE TABLE IF NOT EXISTS TAUTH ( "
        "ID INTEGER NOT NULL PRIMARY KEY " AUTO_INCREMENT ","
        "TERMINAL_ID VARCHAR(10) NOT NULL,"
        "RECEIVED_DATE VARCHAR(8) NOT NULL,"
        "RECEIVED_TIME VARCHAR(6) NOT NULL,"
        "INTERNATIONAL_CALL_IND BOOLEAN,"
        "CALLED_TELEPHONE_NO VARCHAR(20),"
        "CARRIER_XREF_NUMBER TINYINT,"
        "CARD_NUMBER VARCHAR(20),"
        "SERVICE_CODE SMALLINT,"
        "CARD_EXPIRY_DATE VARCHAR(6),"
        "INITIAL_DATE VARCHAR(6),"
        "DISCRETIONARY SMALLINT,"
        "SPARE_FORMATTED VARCHAR(2),"
        "CARD_DATA VARCHAR(40),"
        "PIN VARCHAR(5),"
        "CALL_TYPE TINYINT,"
        "CARD_REF_NO TINYINT,"
        "SEQUENCE_NO SMALLINT,"
        "FOLLOW_ON_IND BOOLEAN,"
        "TELCO_ID VARCHAR(2) DEFAULT 0, REGION_CODE VARCHAR(3) DEFAULT \"USA\", ARCHIVE_IND BOOLEAN DEFAULT 0,"
        "UNIQUE(TERMINAL_ID,RECEIVED_DATE,SEQUENCE_NO) "
        ");");

    if (rc != 0) {
        fprintf(stderr, "%s: Failed to create table TAUTH.\n", __func__);
        return -1;
    }

    rc = mm_sql_exec(db, "CREATE TABLE IF NOT EXISTS TCDR ( "
        "ID INTEGER NOT NULL PRIMARY KEY " AUTO_INCREMENT ","
        "TERMINAL_ID VARCHAR(10) NOT NULL,"
        "RECEIVED_DATE VARCHAR(8) NOT NULL,"
        "RECEIVED_TIME VARCHAR(6) NOT NULL,"
        "SEQ INTEGER NOT NULL,"
        "START_DATE VARCHAR(8) NOT NULL,"
        "START_TIME VARCHAR(6) NOT NULL,"
        "CALL_DURATION INTEGER, "
        "CD_CALL_TYPE TINYINT UNSIGNED, "
        "CD_CALL_TYPE_STR TEXT, "
        "DIALED_NUM VARCHAR(10), "
        "CARD VARCHAR(20), "
        "REQUESTED REAL, "
        "COLLECTED REAL, "
        "CARRIER INTEGER, "
        "RATE INTEGER,"
        "TELCO_ID VARCHAR(2) DEFAULT 0, REGION_CODE VARCHAR(3) DEFAULT \"USA\", ARCHIVE_IND BOOLEAN DEFAULT 0,"
        "UNIQUE(TERMINAL_ID,START_DATE,START_TIME,SEQ) "
        ");");

    if (rc != 0) {
        fprintf(stderr, "%s: Failed to create table TCDR.\n", __func__);
        return -1;
    }

    rc = mm_sql_exec(db, "CREATE TABLE IF NOT EXISTS TCALLST ( "
        "ID INTEGER NOT NULL PRIMARY KEY " AUTO_INCREMENT ","
        "TERMINAL_ID VARCHAR(10) NOT NULL,"
        "RECEIVED_DATE VARCHAR(8) NOT NULL,"
        "RECEIVED_TIME VARCHAR(6) NOT NULL,"
        "SUMMARY_PERIOD_START_DATE VARCHAR(8) NOT NULL,"
        "SUMMARY_PERIOD_START_TIME VARCHAR(6) NOT NULL,"
        "SUMMARY_PERIOD_STOP_DATE VARCHAR(8),"
        "SUMMARY_PERIOD_STOP_TIME VARCHAR(6),"
        "TOTAL_CARD_CALL_CNT SMALLINT,"
        "FREE_CALL_CNT SMALLINT,"
        "INCOMING_CALL_CNT SMALLINT,"
        "UNANSWERED_CALL_CNT SMALLINT,"
        "ABANDONED_CALL_CNT SMALLINT,"
        "LOCAL_CARD_CALL_CNT SMALLINT,"
        "TOLL_CARD_CALL_CNT SMALLINT,"
        "OPERATOR_CALL_CNT SMALLINT,"
        "ZERO_PLUS_CALL_CNT SMALLINT,"
        "FOLLOW_ON_CALL_CNT SMALLINT,"
        "REP_DIALER_PEG_CNT1 SMALLINT,"
        "REP_DIALER_PEG_CNT2 SMALLINT,"
        "REP_DIALER_PEG_CNT3 SMALLINT,"
        "REP_DIALER_PEG_CNT4 SMALLINT,"
        "REP_DIALER_PEG_CNT5 SMALLINT,"
        "REP_DIALER_PEG_CNT6 SMALLINT,"
        "REP_DIALER_PEG_CNT7 SMALLINT,"
        "REP_DIALER_PEG_CNT8 SMALLINT,"
        "REP_DIALER_PEG_CNT9 SMALLINT,"
        "REP_DIALER_PEG_CNT10 SMALLINT,"
        "TOTAL_CALL_DURATION VARCHAR(11),"
        "TOTAL_TIME_OFF_HOOK VARCHAR(11),"
        "TOTAL_COIN_CALL_CNT SMALLINT,"
        "LOCAL_COIN_CALL_CNT SMALLINT,"
        "TOLL_COIN_CALL_CNT SMALLINT,"
        "FAIL_TO_POTS_COIN_CNT SMALLINT,"
        "INTER_LATA_TOLL_CARD_CALL_CNT SMALLINT,"
        "INTER_LATA_TOLL_COIN_CALL_CNT SMALLINT,"
        "SPARE1 SMALLINT,"
        "SPARE2 SMALLINT,"
        "TELCO_ID VARCHAR(2) DEFAULT 0, REGION_CODE VARCHAR(3) DEFAULT \"USA\", ARCHIVE_IND BOOLEAN DEFAULT 0,"
        "UNIQUE(TERMINAL_ID,SUMMARY_PERIOD_START_DATE,SUMMARY_PERIOD_START_TIME,SUMMARY_PERIOD_STOP_DATE,SUMMARY_PERIOD_STOP_TIME) "
        ");");

    if (rc != 0) {
        fprintf(stderr, "%s: Failed to create table TALARM.\n", __func__);
        return -1;
    }

    rc = mm_sql_exec(db, "CREATE TABLE IF NOT EXISTS TCASHST ( "
        "ID INTEGER NOT NULL PRIMARY KEY " AUTO_INCREMENT ","
        "TERMINAL_ID VARCHAR(10) UNIQUE NOT NULL,"
        "RECEIVED_DATE VARCHAR(8) NOT NULL,"
        "RECEIVED_TIME VARCHAR(6) NOT NULL,"
        "START_DATE VARCHAR(8) NOT NULL,"
        "START_TIME VARCHAR(6) NOT NULL,"
        "CASH_BOX_STATUS TINYINT UNSIGNED,"
        "BOX_NUMBER VARCHAR(8),"
        "PERCENT_FULL TINYINT UNSIGNED CHECK (PERCENT_FULL <= 100),"
        "CURRENCY_VALUE REAL DEFAULT \"0.00\","
        "NUMBER_OF_CDN_NICKELS SMALLINT UNSIGNED DEFAULT 0,"
        "NUMBER_OF_CDN_DIMES SMALLINT UNSIGNED DEFAULT 0,"
        "NUMBER_OF_CDN_QUARTERS SMALLINT UNSIGNED DEFAULT 0,"
        "NUMBER_OF_CDN_DOLLARS SMALLINT UNSIGNED DEFAULT 0,"
        "NUMBER_OF_US_NICKELS SMALLINT UNSIGNED DEFAULT 0,"
        "NUMBER_OF_US_DIMES SMALLINT UNSIGNED DEFAULT 0,"
        "NUMBER_OF_US_QUARTERS SMALLINT UNSIGNED DEFAULT 0,"
        "NUMBER_OF_US_DOLLARS SMALLINT UNSIGNED DEFAULT 0,"
        "TELCO_ID VARCHAR(2) DEFAULT 0, REGION_CODE VARCHAR(3) DEFAULT \"USA\", ARCHIVE_IND BOOLEAN DEFAULT 0"
        ");");

    if (rc != 0) {
        fprintf(stderr, "%s: Failed to create table TCASHST.\n", __func__);
        return -1;
    }

    rc = mm_sql_exec(db, "CREATE TABLE IF NOT EXISTS TCOLLST ( "
        "ID INTEGER NOT NULL PRIMARY KEY " AUTO_INCREMENT ","
        "TERMINAL_ID VARCHAR(10) NOT NULL, "
        "RECEIVED_DATE VARCHAR(8) NOT NULL,"
        "RECEIVED_TIME VARCHAR(6) NOT NULL,"
        "COLLECTION_DATE VARCHAR(8) NOT NULL,"
        "COLLECTION_TIME VARCHAR(6) NOT NULL,"
        "CASH_BOX_STATUS TINYINT UNSIGNED,"
        "BOX_NUMBER VARCHAR(8),"
        "PERCENT_FULL TINYINT UNSIGNED CHECK (PERCENT_FULL <= 100),"
        "CURRENCY_VALUE REAL,"
        "NUMBER_OF_CDN_NICKELS SMALLINT UNSIGNED,"
        "NUMBER_OF_CDN_DIMES SMALLINT UNSIGNED,"
        "NUMBER_OF_CDN_QUARTERS SMALLINT UNSIGNED,"
        "NUMBER_OF_CDN_DOLLARS SMALLINT UNSIGNED,"
        "NUMBER_OF_US_NICKELS SMALLINT UNSIGNED,"
        "NUMBER_OF_US_DIMES SMALLINT UNSIGNED,"
        "NUMBER_OF_US_QUARTERS SMALLINT UNSIGNED,"
        "NUMBER_OF_US_DOLLARS SMALLINT UNSIGNED,"
        "TELCO_ID VARCHAR(2) DEFAULT 0, REGION_CODE VARCHAR(3) DEFAULT \"USA\", ARCHIVE_IND BOOLEAN DEFAULT 0,"
        "UNIQUE(TERMINAL_ID,COLLECTION_DATE,COLLECTION_TIME) "
        ");");

    if (rc != 0) {
        fprintf(stderr, "%s: Failed to create table TCASHST.\n", __func__);
        return -1;
    }

   rc = mm_sql_exec(db, "CREATE TABLE IF NOT EXISTS TOPCODE ( "
        "ID INTEGER NOT NULL PRIMARY KEY " AUTO_INCREMENT ","
        "TERMINAL_ID VARCHAR(10) NOT NULL, "
        "RECEIVED_DATE VARCHAR(8) NOT NULL,"
        "RECEIVED_TIME VARCHAR(6) NOT NULL,"
        "OP_CODE INTEGER, "
        "PIN INTEGER,"
        "TELCO_ID VARCHAR(2) DEFAULT 0, REGION_CODE VARCHAR(3) DEFAULT \"USA\", ARCHIVE_IND BOOLEAN DEFAULT 0"
        ");");

    if (rc != 0) {
        fprintf(stderr, "%s: Failed to create table TOPCODE.\n", __func__);
        return -1;
    }

    rc = mm_sql_exec(db, "CREATE TABLE IF NOT EXISTS TPERFST ( "
        "ID INTEGER NOT NULL PRIMARY KEY " AUTO_INCREMENT ","
        "TERMINAL_ID VARCHAR(10) NOT NULL, "
        "RECEIVED_DATE VARCHAR(8) NOT NULL,"
        "RECEIVED_TIME VARCHAR(6) NOT NULL,"
        "SUMMARY_PERIOD_START_DATE DATE,"
        "SUMMARY_PERIOD_START_TIME TIME,"
        "SUMMARY_PERIOD_STOP_DATE DATE,"
        "SUMMARY_PERIOD_STOP_TIME TIME,"
        "CALL_ATTEMPTS_CNT SMALLINT,"
        "BUSY_SIGNAL_CNT SMALLINT,"
        "CALL_CLEARED_NO_DATA SMALLINT,"
        "NO_CARRIER_DETECT_CNT SMALLINT,"
        "CO_ACCESS_TO_DIAL_CNT1 SMALLINT,"
        "CO_ACCESS_TO_DIAL_CNT2 SMALLINT,"
        "CO_ACCESS_TO_DIAL_CNT3 SMALLINT,"
        "CO_ACCESS_TO_DIAL_CNT4 SMALLINT,"
        "CO_ACCESS_TO_DIAL_CNT5 SMALLINT,"
        "CO_ACCESS_TO_DIAL_CNT6 SMALLINT,"
        "CO_ACCESS_TO_DIAL_CNT7 SMALLINT,"
        "DIAL_TO_CARRIER_CNT1 SMALLINT,"
        "DIAL_TO_CARRIER_CNT2 SMALLINT,"
        "DIAL_TO_CARRIER_CNT3 SMALLINT,"
        "DIAL_TO_CARRIER_CNT4 SMALLINT,"
        "DIAL_TO_CARRIER_CNT5 SMALLINT,"
        "DIAL_TO_CARRIER_CNT6 SMALLINT,"
        "DIAL_TO_CARRIER_CNT7 SMALLINT,"
        "CARRIER_TO_1ST_PACKET_CNT1 SMALLINT,"
        "CARRIER_TO_1ST_PACKET_CNT2 SMALLINT,"
        "CARRIER_TO_1ST_PACKET_CNT3 SMALLINT,"
        "CARRIER_TO_1ST_PACKET_CNT4 SMALLINT,"
        "CARRIER_TO_1ST_PACKET_CNT5 SMALLINT,"
        "CARRIER_TO_1ST_PACKET_CNT6 SMALLINT,"
        "CARRIER_TO_1ST_PACKET_CNT7 SMALLINT,"
        "USER_WAIT_TO_EXPECT_INFO_CNT1 SMALLINT,"
        "USER_WAIT_TO_EXPECT_INFO_CNT2 SMALLINT,"
        "USER_WAIT_TO_EXPECT_INFO_CNT3 SMALLINT,"
        "USER_WAIT_TO_EXPECT_INFO_CNT4 SMALLINT,"
        "USER_WAIT_TO_EXPECT_INFO_CNT5 SMALLINT,"
        "USER_WAIT_TO_EXPECT_INFO_CNT6 SMALLINT,"
        "USER_WAIT_TO_EXPECT_INFO_CNT7 SMALLINT,"
        "TOTAL_DIALOGS_FAILED SMALLINT,"
        "NO_PACKET_RCVD_ERRORS SMALLINT,"
        "NO_PACKET_RETRIES_RCVD SMALLINT,"
        "INACTIVITY_COUNT SMALLINT,"
        "RETRY_LIMIT_OUT_OF_SERVICE SMALLINT,"
        "CARD_AUTH_TIMEOUTS SMALLINT,"
        "RATE_REQUEST_TIMEOUTS SMALLINT,"
        "NO_DIAL_TONE SMALLINT,"
        "SPARE1 SMALLINT,"
        "SPARE2 SMALLINT,"
        "SPARE3 SMALLINT,"
        "TELCO_ID VARCHAR(2) DEFAULT 0, REGION_CODE VARCHAR(3) DEFAULT \"USA\", ARCHIVE_IND BOOLEAN DEFAULT 0,"
        "UNIQUE(TERMINAL_ID,SUMMARY_PERIOD_START_DATE,SUMMARY_PERIOD_START_TIME,SUMMARY_PERIOD_STOP_DATE,SUMMARY_PERIOD_STOP_TIME) "
        ");");

    if (rc != 0) {
        fprintf(stderr, "%s: Failed to create table TPERFST.\n", __func__);
        return -1;
    }

    rc = mm_sql_exec(db, "CREATE TABLE IF NOT EXISTS TSTATUS ( "
        "ID INTEGER NOT NULL PRIMARY KEY " AUTO_INCREMENT ","
        "TERMINAL_ID VARCHAR(10) NOT NULL, "
        "RECEIVED_DATE VARCHAR(8) NOT NULL,"
        "RECEIVED_TIME VARCHAR(6) NOT NULL,"
        "SERIAL_NO VARCHAR(10),"
        "STATUS_WORD BIGINT UNSIGNED,"
        "HANDSET_DISCONT_IND BOOLEAN,"
        "TELEPHONY_STATUS_IND BOOLEAN,"
        "EPM_SAM_NOT_RESPONDING BOOLEAN,"
        "EPM_SAM_LOCKED_OUT BOOLEAN,"
        "EPM_SAM_EXPIRED BOOLEAN,"
        "EPM_SAM_REACHING_TRANS_LIMIT BOOLEAN,"
        "UNABLE_REACH_PRIM_COL_SYS BOOLEAN,"
        "TELEPHONY_STATUS_BIT_7 BOOLEAN,"
        "POWER_FAIL_IND BOOLEAN,"
        "DISPLAY_RESPONSE_IND BOOLEAN,"
        "VOICE_SYNTHESIS_RESPONSE_IND BOOLEAN,"
        "UNABLE_REACH_SECOND_COL_SYS BOOLEAN,"
        "CARD_READER_BLOCKED_ALARM BOOLEAN,"
        "MANDATORY_TABLE_ALARM BOOLEAN,"
        "DATAJACK_PORT_BLOCKED BOOLEAN,"
        "CTRL_HW_STATUS_BIT_7 BOOLEAN,"
        "CDR_CHECKSUM_ERR_IND BOOLEAN,"
        "STATISTICS_CHECKSUM_ERR_IND BOOLEAN,"
        "TERMINAL_TBL_CHECKSUM_ERR_IND BOOLEAN,"
        "OTHER_DATA_CHECKSUM_ERR_IND BOOLEAN,"
        "CDR_LIST_FULL_ERR_IND BOOLEAN,"
        "BAD_EEPROM_ERR_IND BOOLEAN,"
        "MEMORY_LOST_ERROR_IND BOOLEAN,"
        "MEMORY_BAD_ERR_IND BOOLEAN,"
        "ACCESS_COVER_IND BOOLEAN,"
        "KEY_MATRIX_MALFUNC_IND BOOLEAN,"
        "SET_REMOVAL_IND BOOLEAN,"
        "THRESHOLD_MET_EXCEEDED_IND BOOLEAN,"
        "CASH_BOX_COVER_OPEN_IND BOOLEAN,"
        "CASH_BOX_REMOVED_IND BOOLEAN,"
        "COIN_BOX_FULL_IND BOOLEAN,"
        "COIN_JAM_COIN_CHUTE_IND BOOLEAN,"
        "ESCROW_JAM_IND BOOLEAN,"
        "VAL_HARDWARE_FAIL_IND BOOLEAN,"
        "CO_LINE_CHECK_FAIL_IND BOOLEAN,"
        "DIALOG_FAILURE_IND BOOLEAN,"
        "CASH_BOX_ELECTRONIC_LOCK_IND BOOLEAN,"
        "DIALOG_FAILURE_WITH_COL_SYS BOOLEAN,"
        "CODE_SERVE_CONNECTION_FAILURE BOOLEAN,"
        "CODE_SERVER_ABORTED BOOLEAN,"
        "TELCO_ID VARCHAR(2) DEFAULT 0, REGION_CODE VARCHAR(3) DEFAULT \"USA\", ARCHIVE_IND BOOLEAN DEFAULT 0"
        ");");

    if (rc != 0) {
        fprintf(stderr, "%s: Failed to create table TSTATUS.\n", __func__);
        return -1;
    }

    rc = mm_sql_exec(db, "CREATE TABLE IF NOT EXISTS TSWVERS ( "
        "ID INTEGER NOT NULL PRIMARY KEY " AUTO_INCREMENT ","
        "TERMINAL_ID VARCHAR(10) NOT NULL, "
        "EFFECTIVE_DATE VARCHAR(8) NOT NULL,"
        "EFFECTIVE_TIME VARCHAR(6) NOT NULL,"
        "CONTROL_ROM_EDITION VARCHAR(7),"
        "CONTROL_VERSION_NO VARCHAR(4),"
        "TELEPHONY_ROM_EDITION VARCHAR(7),"
        "TELEPHONY_VERSION_NO VARCHAR(4),"
        "FEATURE_TERMINAL_TYPE SMALLINT UNSIGNED,"
        "TERMINAL_TYPE SMALLINT UNSIGNED,"
        "VALIDATOR_SOFTWARE_VERS VARCHAR(2),"
        "VALIDATOR_HARDWARE_VERS VARCHAR(2),"
        "TELCO_ID VARCHAR(2) DEFAULT 0, REGION_CODE VARCHAR(3) DEFAULT \"USA\", ARCHIVE_IND BOOLEAN DEFAULT 0,"
        "UNIQUE(CONTROL_ROM_EDITION,CONTROL_VERSION_NO,TELEPHONY_ROM_EDITION,TELEPHONY_VERSION_NO,"
               "FEATURE_TERMINAL_TYPE,TERMINAL_TYPE,VALIDATOR_SOFTWARE_VERS,VALIDATOR_HARDWARE_VERS) "
        ");");

    if (rc != 0) {
        fprintf(stderr, "%s: Failed to create table TSWVERS.\n", __func__);
        return -1;
    }

    return 0;
}
