/*
 * Code to dump Credit Card table from Nortel Millennium Payphone
 * Table 134 (0x86)
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "./mm_manager.h"
#include "./mm_card.h"

const char *standard_cd_str[] = {
    "Undefd",  // 0
    "MOD 10",  // 1
    "ANSI  ",  // 2
    "ABA   ",  // 3
    "CBA   ",  // 4
    "BOC   ",  // 5
    "ANSI59",  // 6
    "CCITT ",  // 7
    "PINOFF",  // 8
    "HELLO ",  // 9
    "SMCARD",  // 10
    "Resv'd",  // 11
    "SCGPM4",  // 12 SmartCity GPM416
    "SCPCOS",  // 13
    "SCMPCO",  // 14
    "PROTON"   // 15
};

const char *str_vfy_flags[] = {
    "MOD10 IND ",  // MOD10_IND
    "NCCVAL IND",  // NCC VALIDATION IND
    "CALLING CD",  // CALLING CARD IND
    "IMMED AUTH",  // IMMEDIATE AUTH IND
    "SVC CD VAL",  // SERVICE CD VALIDATION IND
    "PROMPT PIN",  // PROMPT FOR PIN
    "TELCO PIN ",  // PROMPT FOR TELCO PIN
    "ACCS ROUTE"   // ROUTING
};

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   index;
    int   j;
    int   ret = 0;

    dlog_mt_card_table_mtr1_t *pcard_table;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_card mm_table_16.bin [outputfile.bin]\n");
        return -1;
    }

    printf("Nortel Millennium Credit Card Table 0x16 (22) Dump\n\n");

    pcard_table = (dlog_mt_card_table_mtr1_t *)calloc(1, sizeof(dlog_mt_card_table_mtr1_t));

    if (pcard_table == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_card_table_mtr1_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(pcard_table);
        return -ENOENT;
    }

    if (fread(pcard_table, sizeof(dlog_mt_card_table_mtr1_t), 1, instream) != 1) {
        printf("Error reading CCARD table.\n");
        free(pcard_table);
        fclose(instream);
        return -EIO;
    }

    printf("+-------------------------------------------------------+---------------+\n" \
           "| Idx  | PAN St - End    | STD CD | Vfy | Carrier | Ref | P exp ini dis |\n" \
           "+------+-----------------+--------+-----+---------+-----+---------------+");

    for (index = 0; index < CCARD_MAX_MTR1; index++) {
        card_entry_mtr1_t *pcard = &pcard_table->c[index];

        if (pcard_table->c[index].standard_cd == 0) continue;

        printf("\n|  %2d  | %02x%02x%02x - %02x%02x%02x | %s | x%02x |   0x%02x  | x%02x | P x%02x x%02x x%02x |",
               index,
               pcard->pan_start[0],
               pcard->pan_start[1],
               pcard->pan_start[2],
               pcard->pan_end[0],
               pcard->pan_end[1],
               pcard->pan_end[2],
               standard_cd_str[(pcard->standard_cd) & 0x0F],
               pcard->vfy_flags,
               pcard->carrier_ref,
               pcard->ref_num,
               pcard->p_exp_date,
               pcard->p_init_date,
               pcard->p_disc_data);

        print_bits((pcard->vfy_flags ^ CARD_VF_CALLING_CARD_IND), (char **)str_vfy_flags);

        printf("\n|      | Service Codes: ");

        switch (pcard->standard_cd) {
            case ansi:
            case aba:
            case cba:
            case boc:
            case ansi59:
            case ccitt:

                for (j = 0; j < SVC_CODE_MAX; j++) {
                    printf("%04x,", pcard->svc_code.cc.svc_code[j]);
                }
                printf("Spill: ");

                for (j = 0; j < SPILL_STRING_LEN; j++) {
                    printf("%02x,", pcard->svc_code.cc.spill_string[j]);
                }
                printf(" TC:%02x ",   pcard->svc_code.cc.term_char);
                printf("DI:%02x |\n", pcard->svc_code.cc.discount_index);
                break;
            case smcard:
                printf("Ck Digits: ");

                for (j = 0; j < SC_CHECK_DIGIT_LEN; j++) {
                    printf("%02x,", pcard->svc_code.sc.check_digits[j]);
                }
                printf("     Ck Value: ");

                for (j = 0; j < SC_CHECK_VALUE_LEN; j++) {
                    printf("%02x,", pcard->svc_code.sc.check_value[j]);
                }
                printf(" |\n|      |                Manufacturer: ");

                for (j = 0; j < SC_MANUF_LEN; j++) {
                    printf("%02x,", pcard->svc_code.sc.manufacturer[j]);
                }

                printf("     Discount Index: %02x                 |\n",
                       pcard->svc_code.sc.discount_index);

                break;
            default:

                for (j = 0; j < SERVICE_CODE_LEN; j++) {
                    printf("%02x,", pcard->svc_code.raw[j]);
                }
                printf("         |\n");
                break;
        }
        printf("+------+-----------------+--------+-----+---------+-----+---------------+");
    }

    printf("\n");

    if (argc == 3) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            return -ENOENT;
        }
    }

    /* Update CARD table */
    pcard_table->c[0].carrier_ref  = 0x01;
    pcard_table->c[1].carrier_ref  = 0x02;
    pcard_table->c[2].carrier_ref  = 0x03;
    pcard_table->c[3].carrier_ref  = 0x04;
    pcard_table->c[4].carrier_ref  = 0x05;
    pcard_table->c[5].carrier_ref  = 0x06;
    pcard_table->c[6].carrier_ref  = 0x07;
    pcard_table->c[7].carrier_ref  = 0x08;
    pcard_table->c[8].carrier_ref  = 0x09;
    pcard_table->c[9].carrier_ref  = 0x01;
    pcard_table->c[10].carrier_ref = 0x02;
    pcard_table->c[11].carrier_ref = 0x03;
    pcard_table->c[12].carrier_ref = 0x04;
    pcard_table->c[13].carrier_ref = 0x05;


    for (index = 0; index < CCARD_MAX_MTR1; index++) {
        pcard_table->c[index].vfy_flags &= ~CARD_VF_ACCS_ROUTING;
    }

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(pcard_table, sizeof(dlog_mt_card_table_mtr1_t), 1, ostream) != 1) {
            printf("Error writing output file %s\n", argv[2]);
            ret = -EIO;
        }
        fclose(ostream);
    }

    if (pcard_table != NULL) {
        free(pcard_table);
    }

    return ret;
}
