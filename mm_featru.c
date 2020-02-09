/*
 * Code to dump Feature Configuration table from Nortel Millennium Payphone
 * Table 26 (0x1a) - FEATRU pp. 2-151
 *
 * www.github.com/hharte/mm_manager
 *
 * (c) 2020, Howard M. Harte
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mm_manager.h"

int main(int argc, char *argv[])
{
    FILE *instream;
    dlog_mt_fconfig_opts_t *featru_table;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_featru mm_table_1a.bin\n");
        return (-1);
    }

    instream = fopen(argv[1], "rb");

    printf("Nortel Millennium FEATRU Table (Table 26) Dump\n");

    featru_table = calloc(1, sizeof(dlog_mt_fconfig_opts_t));
    if (fread(featru_table, sizeof(dlog_mt_fconfig_opts_t), 1, instream) <= 0) {
        printf("Error reading ADMESS table.\n");
        if (featru_table != NULL) {
            free(featru_table);
            return (-2);
        }
    }

    printf("                            term_type: 0x%02x\n", featru_table->term_type);
    printf("                      display_present: %d\n", featru_table->display_present);
    printf("                     num_call_follows: %d\n", featru_table->num_call_follows);
    printf("                        card_val_info: 0x%02x\n", featru_table->card_val_info);
    printf("                       accs_mode_info: 0x%02x\n", featru_table->accs_mode_info);
    printf("                   incoming_call_mode: 0x%02x\n", featru_table->incoming_call_mode);
    printf("         anti_fraud_for_incoming_call: 0x%02x\n", featru_table->anti_fraud_for_incoming_call);
    printf("                       OOS_POTS_flags: 0x%02x\n", featru_table->OOS_POTS_flags);
    printf("              datajack_visual_display: 0x%02x\n", featru_table->datajack_visual_display);
    printf("                    lang_scroll_order: 0x%02x\n", featru_table->lang_scroll_order);
    printf("                   lang_scroll_order2: 0x%02x\n", featru_table->lang_scroll_order2);
    printf("                     num_of_languages: %d\n", featru_table->num_of_languages);
    printf("                         rating_flags: 0x%02x\n", featru_table->rating_flags);
    printf("                     dialaround_timer: %d\n", featru_table->dialaround_timer);
    printf("    ptr_international_access_operator: 0x%02x\n", featru_table->ptr_international_access_operator);
    printf("                inter_lata_aos_number: 0x%02x\n", featru_table->inter_lata_aos_number);
    printf("      international_access_aos_number: 0x%02x\n", featru_table->international_access_aos_number);
    printf("                datajack_grace_period: %d\n", featru_table->datajack_grace_period);
    printf("            operator_collection_timer: %d\n", featru_table->operator_collection_timer);
    printf("   ptr_intra_lata_operator_access_num: 0x%02x\n", featru_table->ptr_intra_lata_operator_access_num);
    printf("   ptr_inter_lata_operator_access_num: 0x%02x\n", featru_table->ptr_inter_lata_operator_access_num);
    printf("                       enable_adverts: 0x%02x\n", featru_table->enable_adverts);
    printf("                     default_language: %d\n", featru_table->default_language);
    printf("                display_called_number: %d\n", featru_table->display_called_number);
    printf("                        dtmf_duration: %d\n", featru_table->dtmf_duration);
    printf("                     interdigit_pause: %d\n", featru_table->interdigit_pause);
    printf("             ppu_preauth_credit_limit: %d\n", featru_table->ppu_preauth_credit_limit);
    printf("                coin_calling_features: 0x%02x\n", featru_table->coin_calling_features);
    printf("            coin_call_overtime_period: %d\n", featru_table->coin_call_overtime_period);
    printf("                  coin_call_pots_time: %d\n", featru_table->coin_call_pots_time);
    printf("             international_min_digits: %d\n", featru_table->international_min_digits);
    printf("        default_rate_req_payment_type: %d\n", featru_table->default_rate_req_payment_type);
    printf("     next_call_revalidation_frequency: %d\n", featru_table->next_call_revalidation_frequency);
    printf("              cutoff_on_disc_duration: %d\n", featru_table->cutoff_on_disc_duration);
    printf("       cdr_upload_timer_international: %d\n", featru_table->cdr_upload_timer_international);
    printf("            cdr_upload_timer_domestic: %d\n", featru_table->cdr_upload_timer_domestic);
    printf("           num_perf_stat_dialog_fails: %d\n", featru_table->num_perf_stat_dialog_fails);
    printf("              num_co_line_check_fails: %d\n", featru_table->num_co_line_check_fails);
    printf("       num_alt_ncc_dialog_check_fails: %d\n", featru_table->num_alt_ncc_dialog_check_fails);
    printf("         num_failed_dialogs_until_oos: %d\n", featru_table->num_failed_dialogs_until_oos);
    printf("       num_failed_dialogs_until_alarm: %d\n", featru_table->num_failed_dialogs_until_alarm);
    printf("                      smartcard_flags: 0x%02x\n", featru_table->smartcard_flags);
    printf("     max_num_digits_manual_card_entry: %d\n", featru_table->max_num_digits_manual_card_entry);
    printf("             ptr_intra_aos_access_num: 0x%02x\n", featru_table->ptr_intra_aos_access_num);
    printf("                carrier_reroute_flags: 0x%02x\n", featru_table->carrier_reroute_flags);
    printf("     min_num_digits_manual_card_entry: %d\n", featru_table->min_num_digits_manual_card_entry);
    printf("            max_num_smartcard_inserts: %d\n", featru_table->max_num_smartcard_inserts);
    printf("       max_num_diff_smartcard_inserts: %d\n", featru_table->max_num_diff_smartcard_inserts);
    printf("              ptr_operator_aos_number: 0x%02x\n", featru_table->ptr_operator_aos_number);
    printf("                       datajack_flags: 0x%02x\n", featru_table->datajack_flags);
    printf("             delay_on_hook_card_alarm: %d\n", featru_table->delay_on_hook_card_alarm);
    printf("  delay_on_hook_card_alarm_after_call: %d\n", featru_table->delay_on_hook_card_alarm_after_call);
    printf("               duration_of_card_alarm: %d\n", featru_table->duration_of_card_alarm);
    printf("                card_alarm_on_cadence: %d\n", featru_table->card_alarm_on_cadence);
    printf("               card_alarm_off_cadence: %d\n", featru_table->card_alarm_off_cadence);
    printf("delay_until_card_reader_blocked_alarm: %d\n", featru_table->delay_until_card_reader_blocked_alarm);
    printf("                      settlement_time: %d\n", featru_table->settlement_time);
    printf("                grace_period_domestic: %d\n", featru_table->grace_period_domestic);
    printf("                          ias_timeout: %d\n", featru_table->ias_timeout);
    printf("           grace_period_international: %d\n", featru_table->grace_period_international);
    printf("       settlement_time_datajack_calls: %d\n", featru_table->settlement_time_datajack_calls);

    if (featru_table != NULL) {
        free(featru_table);
    }

    return (0);
}
