/*
 * Code to dump Installation/Servicing table from Nortel Millennium Payphone
 * Table 31 (0x1f) - INSTSV pp. 2-236
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

const char *instsv_flags_bits_str[] = {
    "INSTSV_PREDIAL_ENABLE",
    "INSTSV_PREDIAL_ENABLE_1P",
    "INSTSV_PREDIAL_ENABLE_IXL",
    "INSTSV_PREDIAL_ENABLE_ALL",
    "INSTSV_PREDIAL_ENABLE_PRI_NCC",
    "INSTSV_PREDIAL_ENABLE_SEC_NCC",
    "INSTSV_AMP_ENABLE",
    "INSTSV_RESERVED_7"
};

int main(int argc, char *argv[])
{
    FILE *instream;
    FILE *ostream = NULL;
    dlog_mt_install_params_t *instsv_table;
    char phone_number_string[21];

    printf("Nortel Millennium INSTSV Table 0x1f (31) Dump\n\n");

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_instsv mm_table_1f.bin [outputfile.bin]\n");
        return (-1);
    }

    instsv_table = calloc(1, sizeof(dlog_mt_install_params_t));
    if (instsv_table == NULL) {
        printf("Failed to allocate %lu bytes.\n", (unsigned long)sizeof(dlog_mt_install_params_t));
        return(-2);
    }

    instream = fopen(argv[1], "rb");

    if (fread(instsv_table, sizeof(dlog_mt_install_params_t), 1, instream) <= 0) {
        printf("Error reading INSTSV table.\n");
        free(instsv_table);
        fclose(instream);
        return (-2);
    }

    fclose(instream);

    printf("             Access code: %s\n", phone_num_to_string(phone_number_string, sizeof(phone_number_string), instsv_table->access_code, sizeof(instsv_table->access_code)));
    printf("         Key card number: %s\n", phone_num_to_string(phone_number_string, sizeof(phone_number_string), instsv_table->key_card_number, sizeof(instsv_table->key_card_number)));
    printf("                   flags: 0x%02x\t", instsv_table->flags);
    print_bits(instsv_table->flags, (char **)instsv_flags_bits_str);
    printf("\n");
    printf("         TX Packet Delay: 0x%02x (%dms)\n", instsv_table->tx_packet_delay, instsv_table->tx_packet_delay * 10);
    printf("           RX Packet Gap: 0x%02x (%dms)\n", instsv_table->rx_packet_gap, instsv_table->rx_packet_gap * 10);
    printf("       Retries until OOS: %d\n", instsv_table-> retries_until_oos);
    printf("      Coin service flags: 0x%02x\n", instsv_table->coin_service_flags);
    printf("    Coinbox lock timeout: %d seconds\n", instsv_table->coinbox_lock_timeout);
    printf("          Predial string: %s\n", callscrn_num_to_string(phone_number_string, sizeof(phone_number_string), instsv_table->predial_string, sizeof(instsv_table->predial_string)));
    printf("Alternate Predial string: %s\n", callscrn_num_to_string(phone_number_string, sizeof(phone_number_string), instsv_table->predial_string_alt, sizeof(instsv_table->predial_string_alt)));

    /* Modify FEATRU table */
    instsv_table->coin_service_flags = INSTSV_CASHBOX_QUERY_MENU_ENABLE;

    if (argc > 2) {
        ostream = fopen(argv[2], "wb");
    }

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);
        fwrite(instsv_table, sizeof(dlog_mt_install_params_t), 1, ostream);
        fclose(ostream);
    }

    if (instsv_table != NULL) {
        free(instsv_table);
    }

    return (0);
}
