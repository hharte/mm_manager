/*
 * Code to dump DLOG_MT_FCONFIG_OPTS table from Nortel Millennium Payphone
 * Table 26 (0x1a) - FEATRU pp. 2-151
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2023, Howard M. Harte
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "./mm_manager.h"

#define TABLE_ID    DLOG_MT_FCONFIG_OPTS
const char *auth_bits_str[] = {
    "FC_CARD_AUTH_ON_LOCAL_CALLS",
    "FC_DELAYED_CARD_AUTHORIZATION",
    "FC_CARD_AUTH_ON_MCE_LOCAL_CALLS",
    "FC_NO_NPA_ADDED_ZP_LOCAL_ACCS",
    "FC_CARD_AUTH_BIT_4",
    "FC_CARD_AUTH_BIT_5",
    "FC_CARD_AUTH_BIT_6",
    "FC_IMMED_MCE_CARD_AUTH"
};

const char *accs_bits_str[] = {
    "FC_ACCS_AVAILABLE",
    "FC_MCE_ROUTING",
    "FC_MANUAL_DIALED_CARD_NUM_ENABLED",
    "FC_MANUALLY_DIALED_NCC_VALID_REQ",
    "FC_AOS_ENABLED",
    "FC_ZERO_PLUS_LOCAL_CALLS_TO_NCC",
    "FC_ACCS_INFO_BIT_6",
    "FC_REMOVE_NPA_ZP_LOCAL_NCC_CALLS"
};

const char *call_mode_str[] = {
    "FC_CALL_MODE_NO_INCOMING",
    "FC_CALL_MODE_INCOMING_VOICE_ONLY",
    "FC_CALL_MODE_RING_DISABLED_ANSWER_DATA",
    "FC_CALL_MODE_RING_ENABLED_ANSWER_DATA"
};

const char *misc_flags_str[] = {
    "FC_IN_SERVICE_ON_CDR_LIST_FULL",
    "FC_TERM_RATE_DISPLAY_OPTION",
    "FC_INCOMING_CALL_FCA_PRECEDENCE",
    "FC_FCA_ON_CARD",
    "FC_REVERT_TO_PRIMARY_NCC_NUM",
    "FC_BLOCK_NO_RATE_CARRIER",
    "FC_RATED_CREDIT_CARD_CDR",
    "FC_11_DIGIT_LOCAL_CALLS"
};

const char *rating_flags_str[] = {
    "FC_ENABLE_NPA_SBR",
    "FC_ENABLE_IXL_SBR",
    "FC_ENABLE_DIAL_AROUND",
    "FC_SHOW_INIT_ADDL_RATE",
    "FC_ROUND_UP_CHARGE",
    "FC_7_DIGIT_NO_WAIT",
    "FC_RATING_BIT6",
    "FC_RATING_BIT7"
};

const char *advertising_bits_str[] = {
    "FC_ADVERT_ENABLED",
    "FC_REP_DIALER_ADVERTISING",
    "FC_CALL_ESTABLISHED_ADVERTISING",
    "FC_ENABLE_DATE_TIME_DISPLAY",
    "FC_TIME_FORMAT",
    "FC_ADVERTISING_FLAGS_BIT_5",
    "FC_ADVERTISING_FLAGS_BIT_6",
    "FC_ADVERTISING_FLAGS_BIT_7"
};

const char *call_setup_flags_str[] = {
    "FC_DISPLAY_CALLED_NUMBER",
    "FC_ENABLE_SERVLEV_DISP_FLASHING",
    "FC_CALL_SETUP_PARAMS_BIT_2",
    "FC_CALL_SETUP_PARAMS_BIT_3",
    "FC_CALL_SETUP_PARAMS_BIT_4",
    "FC_CALL_SETUP_PARAMS_BIT_5",
    "FC_CALL_SETUP_PARAMS_BIT_6",
    "FC_SUPPRESS_CALLING_PROMPT"
};

const char *coin_calling_features_str[] = {
    "FC_COIN_CALL_OVERTIME",
    "FC_VOICE_FEEDBACK_ON_COIN_CALL",
    "FC_COIN_CALL_SECOND_WARNING",
    "FC_COIN_CALL_FEATURES_BIT_3",
    "FC_COIN_CALL_FEATURES_BIT_4",
    "FC_COIN_CALL_FEATURES_BIT_5",
    "FC_COIN_CALL_FEATURES_BIT_6",
    "FC_COIN_CALL_FEATURES_BIT_7"
};

const char *smartcard_flags_str[] = {
    "FC_SMART_CARD_FLAGS_BIT_0",
    "FC_SC_VALID_INTERNATIONAL_CALLS",
    "FC_SC_VALID_INTER_LATA_CALLS",
    "FC_SC_VALID_INTRA_LATA_CALLS",
    "FC_SC_VALID_LOCAL_CALLS",
    "FC_POST_PAYMENT_RATE_REQUEST",
    "FC_USE_TERMINAL_CARD_TABLE_DEF",
    "FC_RATE_INFO_NOT_DISPLAYED",
};

const char *carrier_reroute_flags_str[] = {
    "FC_BLOCK_REROUTE_COIN_CALL",
    "FC_BLOCK_REROUTE_CREDIT_CARD_CALL",
    "FC_BLOCK_REROUTE_SMART_CARD_CALL",
    "FC_BLOCK_REROUTE_CALL_CARD_CALL",
    "FC_CARRIER_BLOCK_REROUTE_BIT_4",
    "FC_CARRIER_BLOCK_REROUTE_BIT_5",
    "FC_CARRIER_BLOCK_REROUTE_BIT_6",
    "FC_CARRIER_BLOCK_REROUTE_BIT_7",
};

const char *datajack_flags_str[] = {
    "FC_DATAJACK_ENABLED",
    "FC_DATAJACK_MUTING",
    "FC_DATAJACK_ALLOW_FREE_LOCAL_CALL",
    "FC_DATAJACK_ALLOW_DA_CALLS",
    "FC_DJ_FLAGS_BIT_4",
    "FC_DJ_FLAGS_BIT_5",
    "FC_DJ_FLAGS_BIT_6",
    "FC_DJ_FLAGS_BIT_7"
};

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    dlog_mt_fconfig_opts_t *ptable;
    uint8_t* load_buffer;
    int ret = 0;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_fconfig mm_table_%02x.bin [outputfile.bin]\n", TABLE_ID);
        return -1;
    }

    printf("Nortel Millennium %s Table %d (0x%02x) Dump\n\n", table_to_string(TABLE_ID), TABLE_ID, TABLE_ID);

    ptable = (dlog_mt_fconfig_opts_t *)calloc(1, sizeof(dlog_mt_fconfig_opts_t));

    if (ptable == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_fconfig_opts_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(ptable);
        return -ENOENT;
    }

    load_buffer = ((uint8_t*)ptable) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_fconfig_opts_t) - 1, 1, instream) != 1) {
        printf("Error reading %s table.\n", table_to_string(TABLE_ID));
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    printf("                             term_type: 0x%02x\n", ptable->term_type);
    printf("                       display_present: %d\n",     ptable->display_present);
    printf("                      num_call_follows: %d\n",     ptable->num_call_follows);
    printf("                         card_val_info: 0x%02x\t", ptable->card_val_info);
    print_bits(ptable->card_val_info, (char **)auth_bits_str);
    printf("\n");
    printf("                        accs_mode_info: 0x%02x\t", ptable->accs_mode_info);
    print_bits(ptable->accs_mode_info, (char **)accs_bits_str);
    printf("\n");
    printf("                    incoming_call_mode: 0x%02x\t%s\n",
           ptable->incoming_call_mode, call_mode_str[(ptable->incoming_call_mode) & 0x3]);
    printf("          anti_fraud_for_incoming_call: 0x%02x\n", ptable->anti_fraud_for_incoming_call);
    printf("                        OOS_POTS_flags: 0x%02x\t", ptable->OOS_POTS_flags);
    print_bits(ptable->OOS_POTS_flags, (char **)misc_flags_str);
    printf("\n");
    printf("                datajack_display_delay: %ds\n",    ptable->datajack_display_delay);
    printf("                     lang_scroll_order: 0x%02x\n", ptable->lang_scroll_order);
    printf("                    lang_scroll_order2: 0x%02x\n", ptable->lang_scroll_order2);
    printf("                      num_of_languages: %d\n",     ptable->num_of_languages);
    printf("                          rating_flags: 0x%02x\t", ptable->rating_flags);
    print_bits(ptable->rating_flags, (char **)rating_flags_str);
    printf("\n");
    printf("                      dialaround_timer: %d\n",     ptable->dialaround_timer);
    printf("       call_screen_list_ixl_oper_entry: 0x%02x\n", ptable->call_screen_list_ixl_oper_entry);
    printf(" call_screen_list_inter_lata_aos_entry: 0x%02x\n", ptable->call_screen_list_inter_lata_aos_entry);
    printf("        call_screen_list_ixl_aos_entry: 0x%02x\n", ptable->call_screen_list_ixl_aos_entry);
    printf("                 datajack_grace_period: %d\n",     ptable->datajack_grace_period);
    printf("             operator_collection_timer: %d\n",     ptable->operator_collection_timer);
    printf("call_screen_list_intra_lata_oper_entry: 0x%02x\n", ptable->call_screen_list_intra_lata_oper_entry);
    printf("call_screen_list_inter_lata_oper_entry: 0x%02x\n", ptable->call_screen_list_inter_lata_oper_entry);
    printf("                     advertising_flags: 0x%02x\t", ptable->advertising_flags);
    print_bits(ptable->advertising_flags, (char **)advertising_bits_str);
    printf("\n");
    printf("                      default_language: %d\n",     ptable->default_language);
    printf("                call_setup_param_flags: 0x%02x\t", ptable->call_setup_param_flags);
    print_bits(ptable->call_setup_param_flags, (char **)call_setup_flags_str);
    printf("\n");
    printf("                         dtmf_duration: %d (%dms)\n",
           ptable->dtmf_duration, ptable->dtmf_duration * 10);
    printf("                      interdigit_pause: %d (%dms)\n",
           ptable->interdigit_pause, ptable->interdigit_pause * 10);
    printf("              ppu_preauth_credit_limit: %d\n",     ptable->ppu_preauth_credit_limit);
    printf("                 coin_calling_features: 0x%02x\t", ptable->coin_calling_features);
    print_bits(ptable->coin_calling_features, (char **)coin_calling_features_str);
    printf("\n");
    printf("             coin_call_overtime_period: %ds\n",    ptable->coin_call_overtime_period);
    printf("                   coin_call_pots_time: %ds\n",    ptable->coin_call_pots_time);
    printf("              international_min_digits: %d\n",     ptable->international_min_digits);
    printf("         default_rate_req_payment_type: %d\n",     ptable->default_rate_req_payment_type);
    printf("      next_call_revalidation_frequency: %d\n",     ptable->next_call_revalidation_frequency);
    printf("               cutoff_on_disc_duration: %d (%dms)\n",
           ptable->cutoff_on_disc_duration, ptable->cutoff_on_disc_duration * 10);
    printf("        cdr_upload_timer_international: %ds\n",    ptable->cdr_upload_timer_international);
    printf("             cdr_upload_timer_domestic: %ds\n",    ptable->cdr_upload_timer_domestic);
    printf("            num_perf_stat_dialog_fails: %d\n",     ptable->num_perf_stat_dialog_fails);
    printf("               num_co_line_check_fails: %d\n",     ptable->num_co_line_check_fails);
    printf("        num_alt_ncc_dialog_check_fails: %d\n",     ptable->num_alt_ncc_dialog_check_fails);
    printf("          num_failed_dialogs_until_oos: %d\n",     ptable->num_failed_dialogs_until_oos);
    printf("        num_failed_dialogs_until_alarm: %d\n",     ptable->num_failed_dialogs_until_alarm);
    printf("                       smartcard_flags: 0x%02x\t", ptable->smartcard_flags);
    print_bits(ptable->smartcard_flags, (char **)smartcard_flags_str);
    printf("\n");
    printf("      max_num_digits_manual_card_entry: %d\n",     ptable->max_num_digits_manual_card_entry);
    printf("         call_screen_list_zp_aos_entry: 0x%02x\n", ptable->call_screen_list_zp_aos_entry);
    printf("                 carrier_reroute_flags: 0x%02x\t", ptable->carrier_reroute_flags);
    print_bits(ptable->carrier_reroute_flags, (char **)carrier_reroute_flags_str);
    printf("\n");
    printf("      min_num_digits_manual_card_entry: %d\n",     ptable->min_num_digits_manual_card_entry);
    printf("             max_num_smartcard_inserts: %d\n",     ptable->max_num_smartcard_inserts);
    printf("        max_num_diff_smartcard_inserts: %d\n",     ptable->max_num_diff_smartcard_inserts);
    printf("         call_screen_list_zm_aos_entry: 0x%02x\n", ptable->call_screen_list_zm_aos_entry);
    printf("                        datajack_flags: 0x%02x\t", ptable->datajack_flags);
    print_bits(ptable->datajack_flags, (char **)datajack_flags_str);
    printf("\n");
    printf("              delay_on_hook_card_alarm: %d\n", ptable->delay_on_hook_card_alarm);
    printf("   delay_on_hook_card_alarm_after_call: %d\n", ptable->delay_on_hook_card_alarm_after_call);
    printf("                duration_of_card_alarm: %d\n", ptable->duration_of_card_alarm);
    printf("                 card_alarm_on_cadence: %d\n", ptable->card_alarm_on_cadence);
    printf("                card_alarm_off_cadence: %d\n", ptable->card_alarm_off_cadence);
    printf("       card_reader_blocked_alarm_delay: %d\n", ptable->card_reader_blocked_alarm_delay);
    printf("                       settlement_time: %d\n", ptable->settlement_time);
    printf("                 grace_period_domestic: %d\n", ptable->grace_period_domestic);
    printf("                           ias_timeout: %d\n", ptable->ias_timeout);
    printf("            grace_period_international: %d\n", ptable->grace_period_international);
    printf("        settlement_time_datajack_calls: %d\n", ptable->settlement_time_datajack_calls);

    /* Modify FEATRU table */
    ptable->card_val_info |= FC_NO_NPA_ADDED_ZP_LOCAL_ACCS;
    ptable->accs_mode_info = 0;

    /* Don't dial a digit when incoming call is answered. */
    ptable->anti_fraud_for_incoming_call = 0;
    ptable->OOS_POTS_flags               = (FC_IN_SERVICE_ON_CDR_LIST_FULL | FC_TERM_RATE_DISPLAY_OPTION |                    \
                                                  FC_INCOMING_CALL_FCA_PRECEDENCE | FC_FCA_ON_CARD | FC_REVERT_TO_PRIMARY_NCC_NUM | \
                                                  FC_RATED_CREDIT_CARD_CDR);  // FC_11_DIGIT_LOCAL_CALLS
    ptable->rating_flags = (FC_ENABLE_NPA_SBR | FC_ENABLE_IXL_SBR | FC_SHOW_INIT_ADDL_RATE | \
                                  FC_ENABLE_DIAL_AROUND);                     // FC_ROUND_UP_CHARGE FC_7_DIGIT_NO_WAIT
    ptable->advertising_flags                    |= FC_REP_DIALER_ADVERTISING;
    ptable->advertising_flags                    &= ~FC_TIME_FORMAT;    /* 24-hour time format. */
    ptable->call_setup_param_flags               |= FC_DISPLAY_CALLED_NUMBER;
    ptable->call_setup_param_flags               &= ~FC_SUPPRESS_CALLING_PROMPT;
    ptable->datajack_flags                       &= ~FC_DATAJACK_ENABLED;
    ptable->grace_period_domestic                 = 5; /* 5 second grace period. */
    ptable->call_screen_list_ixl_aos_entry        = 0;
    ptable->call_screen_list_inter_lata_aos_entry = 0;
    ptable->call_screen_list_zm_aos_entry         = 0;
    ptable->call_screen_list_zp_aos_entry         = 0;

    if (argc > 2) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            free(ptable);
            return -ENOENT;
        }
    }

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(load_buffer, sizeof(dlog_mt_fconfig_opts_t) - 1, 1, ostream) != 1) {
            printf("Error writing output file %s\n", argv[2]);
            ret = -EIO;
        }
        fclose(ostream);
    }

    if (ptable != NULL) {
        free(ptable);
    }

    return ret;
}
