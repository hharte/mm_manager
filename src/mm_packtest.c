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
#include <assert.h>

#include "mm_manager.h"

#define printf_sizeof(s) printf("assert(sizeof(%s) == %zu);\n", #s, sizeof(s));

int main(int argc, char *argv[]) {
    int   ret = 0;

    (void)argc;
    (void)argv;
    printf("Nortel Millennium data structure Packing Test\n\n");
    printf_sizeof(dlog_mt_advert_prompts_t);
    printf_sizeof(dlog_mt_alarm_t);
    printf_sizeof(dlog_mt_auth_resp_code_t);
    printf_sizeof(dlog_mt_call_back_t);
    printf_sizeof(dlog_mt_call_back_req_t);
    printf_sizeof(dlog_mt_call_details_t);
    printf_sizeof(dlog_mt_call_in_t);
    printf_sizeof(dlog_mt_call_in_params_t);
    printf_sizeof(dlog_mt_call_screen_enhanced_t);
    printf_sizeof(dlog_mt_call_screen_list_t);
    printf_sizeof(dlog_mt_call_screen_universal_t);
    printf_sizeof(dlog_mt_call_stat_params_t);
    printf_sizeof(carrier_stats_entry_t);
    printf_sizeof(dlog_mt_carrier_call_stats_t);
    printf_sizeof(carrier_stats_exp_entry_t);
    printf_sizeof(dlog_mt_carrier_stats_exp_t);
    printf_sizeof(carrier_table_entry_t);
    printf_sizeof(dlog_mt_carrier_table_t);
    printf_sizeof(carrier_table_entry_mtr1_t);
    printf_sizeof(dlog_mt_carrier_table_mtr1_t);
    printf_sizeof(dlog_mt_cash_box_collection_t);
    printf_sizeof(dlog_mt_coin_val_table_t);
    printf_sizeof(dlog_mt_comm_stat_params_t);
    printf_sizeof(dlog_mt_compressed_lcd_table_t);
    printf_sizeof(dlog_mt_end_data_t);
    printf_sizeof(dlog_mt_fconfig_opts_t);
    printf_sizeof(dlog_mt_funf_card_auth_t);
    printf_sizeof(dlog_mt_install_params_t);
    printf_sizeof(dlog_mt_intl_sbr_table_t);
    printf_sizeof(dlog_mt_lcd_table_t);
    printf_sizeof(dlog_mt_limserv_data_t);
    printf_sizeof(dlog_mt_maint_req_t);
    printf_sizeof(dlog_mt_ncc_term_params_t);
    printf_sizeof(dlog_mt_ncc_term_params_mtr1_t);
    printf_sizeof(dlog_mt_npa_nxx_table_t);
    printf_sizeof(dlog_mt_npa_sbr_table_t);
    printf_sizeof(dlog_mt_perf_stats_record_t);
    printf_sizeof(dlog_mt_query_term_err_t);
    printf_sizeof(dlog_mt_rate_request_t);
    printf_sizeof(dlog_mt_rate_response_t);
    printf_sizeof(intl_rate_table_entry_t);
    printf_sizeof(rate_table_entry_t);
    printf_sizeof(dlog_mt_rate_table_t);
    printf_sizeof(dlog_mt_rdlist_table_t);
    printf_sizeof(dlog_mt_scard_parm_table_t);
    printf_sizeof(dlog_mt_summary_call_stats_t);
    printf_sizeof(dlog_mt_sw_version_t);
    printf_sizeof(dlog_mt_table_upd_t);
    printf_sizeof(dlog_mt_term_status_t);
    printf_sizeof(cashbox_status_univ_t);
    printf_sizeof(dlog_mt_time_sync_t);
    printf_sizeof(dlog_mt_time_sync_req_t);
    printf_sizeof(dlog_mt_trans_data_t);
    printf_sizeof(dlog_mt_user_if_params_t);

    assert(sizeof(dlog_mt_advert_prompts_t) == 481);
    assert(sizeof(dlog_mt_alarm_t) == 8);
    assert(sizeof(dlog_mt_auth_resp_code_t) == 22);
    assert(sizeof(dlog_mt_call_back_t) == 1);
    assert(sizeof(dlog_mt_call_back_req_t) == 7);
    assert(sizeof(dlog_mt_call_details_t) == 54);
    assert(sizeof(dlog_mt_call_in_t) == 1);
    assert(sizeof(dlog_mt_call_in_params_t) == 21);
    assert(sizeof(dlog_mt_call_screen_enhanced_t) == 1021);
    assert(sizeof(dlog_mt_call_screen_list_t) == 3061);
    assert(sizeof(dlog_mt_call_screen_universal_t) == 721);
    assert(sizeof(dlog_mt_call_stat_params_t) == 19);
    assert(sizeof(carrier_stats_entry_t) == 59);
    assert(sizeof(dlog_mt_carrier_call_stats_t) == 190);
    assert(sizeof(carrier_stats_exp_entry_t) == 115);
    assert(sizeof(dlog_mt_carrier_stats_exp_t) == 244);
    assert(sizeof(carrier_table_entry_t) == 33);
    assert(sizeof(dlog_mt_carrier_table_t) == 1109);
    assert(sizeof(carrier_table_entry_mtr1_t) == 32);
    assert(sizeof(dlog_mt_carrier_table_mtr1_t) == 678);
    assert(sizeof(dlog_mt_cash_box_collection_t) == 71);
    assert(sizeof(dlog_mt_coin_val_table_t) == 104);
    assert(sizeof(dlog_mt_comm_stat_params_t) == 33);
    assert(sizeof(dlog_mt_compressed_lcd_table_t) == 403);
    assert(sizeof(dlog_mt_end_data_t) == 1);
    assert(sizeof(dlog_mt_fconfig_opts_t) == 72);
    assert(sizeof(dlog_mt_funf_card_auth_t) == 39);
    assert(sizeof(dlog_mt_install_params_t) == 37);
    assert(sizeof(dlog_mt_intl_sbr_table_t) == 604);
    assert(sizeof(dlog_mt_lcd_table_t) == 819);
    assert(sizeof(dlog_mt_limserv_data_t) == 26);
    assert(sizeof(dlog_mt_maint_req_t) == 6);
    assert(sizeof(dlog_mt_ncc_term_params_t) == 48);
    assert(sizeof(dlog_mt_ncc_term_params_mtr1_t) == 18);
    assert(sizeof(dlog_mt_npa_nxx_table_t) == 203);
    assert(sizeof(dlog_mt_npa_sbr_table_t) == 401);
    assert(sizeof(dlog_mt_perf_stats_record_t) == 99);
    assert(sizeof(dlog_mt_query_term_err_t) == 1);
    assert(sizeof(dlog_mt_rate_request_t) == 25);
    assert(sizeof(dlog_mt_rate_response_t) == 26);
    assert(sizeof(intl_rate_table_entry_t) == 3);
    assert(sizeof(rate_table_entry_t) == 9);
    assert(sizeof(dlog_mt_rate_table_t) == 1192);
    assert(sizeof(dlog_mt_rdlist_table_t) == 571);
    assert(sizeof(dlog_mt_scard_parm_table_t) == 225);
    assert(sizeof(dlog_mt_summary_call_stats_t) == 81);
    assert(sizeof(dlog_mt_sw_version_t) == 28);
    assert(sizeof(dlog_mt_table_upd_t) == 1);
    assert(sizeof(dlog_mt_term_status_t) == 11);
    assert(sizeof(cashbox_status_univ_t) == 57);
    assert(sizeof(dlog_mt_time_sync_t) == 8);
    assert(sizeof(dlog_mt_time_sync_req_t) == 1);
    assert(sizeof(dlog_mt_trans_data_t) == 1);
    assert(sizeof(dlog_mt_user_if_params_t) == 68);

    return ret;
}
