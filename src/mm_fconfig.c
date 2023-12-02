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

#include "mm_manager.h"

#define TABLE_ID    DLOG_MT_FCONFIG_OPTS

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

    if (mm_validate_table_fsize(TABLE_ID, instream, sizeof(dlog_mt_fconfig_opts_t) - 1) != 0) {
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    load_buffer = ((uint8_t*)ptable) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_fconfig_opts_t) - 1, 1, instream) != 1) {
        printf("Error reading %s table.\n", table_to_string(TABLE_ID));
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    print_fconfig_table(ptable);

    /* Modify FEATRU table */
    ptable->card_val_info |= FC_NO_NPA_ADDED_ZP_LOCAL_ACCS;
    ptable->accs_mode_info = 0;

    /* Don't dial a digit when incoming call is answered. */
    ptable->anti_fraud_for_incoming_call = 0;
    ptable->OOS_POTS_flags               = (FC_IN_SERVICE_ON_CDR_LIST_FULL | FC_TERM_RATE_DISPLAY_OPTION |                    \
                                            FC_INCOMING_CALL_FCA_PRECEDENCE | FC_FCA_ON_CARD | FC_REVERT_TO_PRIMARY_NCC_NUM | \
                                            FC_RATED_CREDIT_CARD_CDR);  // FC_11_DIGIT_LOCAL_CALLS
    ptable->rating_flags                 = (FC_ENABLE_NPA_SBR | FC_ENABLE_IXL_SBR | FC_SHOW_INIT_ADDL_RATE | \
                                            FC_ENABLE_DIAL_AROUND);                     // FC_ROUND_UP_CHARGE FC_7_DIGIT_NO_WAIT
    ptable->advertising_flags           |=  FC_REP_DIALER_ADVERTISING;
    ptable->advertising_flags           &= ~FC_TIME_FORMAT;    /* 24-hour time format. */
    ptable->call_setup_param_flags      |=  FC_DISPLAY_CALLED_NUMBER;
    ptable->call_setup_param_flags      &= ~FC_SUPPRESS_CALLING_PROMPT;
    ptable->datajack_flags              &= ~FC_DATAJACK_ENABLED;
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
