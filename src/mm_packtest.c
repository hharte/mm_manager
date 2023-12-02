/*
 * Code to check structure packing for mm_manager.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2023, Howard M. Harte
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "mm_manager.h"

int main(int argc, char *argv[]) {
    int   ret = 0;

    (void)argc;
    (void)argv;

    printf("Sizeof                dlog_mt_alarm_t: %zu\n", sizeof(dlog_mt_alarm_t));
    printf("Sizeof            dlog_mt_maint_req_t: %zu\n", sizeof(dlog_mt_maint_req_t));
    printf("Sizeof        dlog_mt_call_back_req_t: %zu\n", sizeof(dlog_mt_call_back_req_t));
    printf("Sizeof      dlog_mt_ncc_term_params_t: %zu\n", sizeof(dlog_mt_ncc_term_params_t));
    printf("Sizeof dlog_mt_ncc_term_params_mtr1_t: %zu\n", sizeof(dlog_mt_ncc_term_params_mtr1_t));
    printf("Sizeof       dlog_mt_call_in_params_t: %zu\n", sizeof(dlog_mt_call_in_params_t));
    printf("Sizeof     dlog_mt_call_stat_params_t: %zu\n", sizeof(dlog_mt_call_stat_params_t));
    printf("Sizeof     dlog_mt_comm_stat_params_t: %zu\n", sizeof(dlog_mt_comm_stat_params_t));
    printf("Sizeof       dlog_mt_user_if_params_t: %zu\n", sizeof(dlog_mt_user_if_params_t));
    printf("Sizeof         dlog_mt_call_details_t: %zu\n", sizeof(dlog_mt_call_details_t));
    printf("Sizeof  dlog_mt_cash_box_collection_t: %zu\n", sizeof(dlog_mt_cash_box_collection_t));
    printf("Sizeof          cashbox_status_univ_t: %zu\n", sizeof(cashbox_status_univ_t));
    printf("Sizeof    dlog_mt_perf_stats_record_t: %zu\n", sizeof(dlog_mt_perf_stats_record_t));
    printf("Sizeof   dlog_mt_summary_call_stats_t: %zu\n", sizeof(dlog_mt_summary_call_stats_t));
    printf("Sizeof          carrier_stats_entry_t: %zu\n", sizeof(carrier_stats_entry_t));
    printf("Sizeof   dlog_mt_carrier_call_stats_t: %zu\n", sizeof(dlog_mt_carrier_call_stats_t));
    printf("Sizeof      carrier_stats_exp_entry_t: %zu\n", sizeof(carrier_stats_exp_entry_t));
    printf("Sizeof    dlog_mt_carrier_stats_exp_t: %zu\n", sizeof(dlog_mt_carrier_stats_exp_t));
    printf("Sizeof           dlog_mt_sw_version_t: %zu\n", sizeof(dlog_mt_sw_version_t));
    printf("Sizeof              dlog_mt_call_in_t: %zu\n", sizeof(dlog_mt_call_in_t));
    printf("Sizeof            dlog_mt_call_back_t: %zu\n", sizeof(dlog_mt_call_back_t));

    printf("Sizeof            dlog_mt_time_sync_t: %zu\n", sizeof(dlog_mt_time_sync_t));
    printf("Sizeof        intl_rate_table_entry_t: %zu\n", sizeof(intl_rate_table_entry_t));
    printf("Sizeof       dlog_mt_intl_sbr_table_t: %zu\n", sizeof(dlog_mt_intl_sbr_table_t));
    printf("Sizeof             rate_table_entry_t: %zu\n", sizeof(rate_table_entry_t));
    printf("Sizeof           dlog_mt_rate_table_t: %zu\n", sizeof(dlog_mt_rate_table_t));
    printf("Sizeof         dlog_mt_rate_request_t: %zu\n", sizeof(dlog_mt_rate_request_t));
    printf("Sizeof        dlog_mt_rate_response_t: %zu\n", sizeof(dlog_mt_rate_response_t));
    printf("Sizeof       dlog_mt_funf_card_auth_t: %zu\n", sizeof(dlog_mt_funf_card_auth_t));
    printf("Sizeof       dlog_mt_auth_resp_code_t: %zu\n", sizeof(dlog_mt_auth_resp_code_t));
    printf("Sizeof          carrier_table_entry_t: %zu\n", sizeof(carrier_table_entry_t));
    printf("Sizeof        dlog_mt_carrier_table_t: %zu\n", sizeof(dlog_mt_carrier_table_t));
    printf("Sizeof     carrier_table_entry_mtr1_t: %zu\n", sizeof(carrier_table_entry_mtr1_t));
    printf("Sizeof   dlog_mt_carrier_table_mtr1_t: %zu\n", sizeof(dlog_mt_carrier_table_mtr1_t));

    return ret;
}
