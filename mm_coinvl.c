/*
 * Code to dump COINVL table from Nortel Millennium Payphone
 * Table 50 (0x32)
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 *
 * Reference: https://wiki.millennium.management/dlog:dlog_mt_carrier_table
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "./mm_manager.h"

const char *str_coin_name[] = {
    "CDN Nickel       ",
    "CDN Nickel2      ",
    "CDN Dime         ",
    "CDN Quarter      ",
    "CDN Dollar       ",
    "US Nickel        ",
    "US Dime          ",
    "US Quarter       ",
    "US Dollar        ",
    "CDN Steel Nickel ",
    "CDN Steel Dime   ",
    "CDN Steel Quarter",
    "Coin 13          ",
    "New CDN Dollar   ",
    "Coin 15          ",
    "Coin 16          "
};

enum coin_val_index {
    cdn_nickel = 0,
    cdn_nickel2,
    cdn_dime,
    cdn_quarter,
    cdn_dollar,
    us_nickel,
    us_dime,
    us_quarter,
    us_dollar,
    cdn_steel_nickel,
    cdn_steel_dime,
    cdn_steel_quarter,
    coin_13,
    cdn_dollar2,
    coin_15,
    coin_16
};

#define COIN_TYPES_MAX  16
#pragma pack(push)
#pragma pack(1) /* Pack data structures for communication with terminal. */

/* DLOG_MT_COIN_VAL_TABLE - COINVL (Coin Validation Parameters) pp. 2-79 */
typedef struct dlog_mt_coin_val_table {
    uint16_t coin_value[COIN_TYPES_MAX];
    uint16_t coin_volume[COIN_TYPES_MAX];
    uint8_t  coin_param[COIN_TYPES_MAX];
    uint16_t cash_box_volume;
    uint16_t escrow_volume;
    uint16_t cash_box_volume_threshold;
    uint32_t cash_box_value_threshold;
    uint16_t escrow_volume_threshold;
    uint32_t escrow_value_threshold;
    uint8_t  pad[8];
} dlog_mt_coin_val_table_t;

#pragma pack(pop)

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   coin_index;
    int   ret = 0;

    dlog_mt_coin_val_table_t *pcoinvl_table;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_coinvl mm_table_32.bin [outputfile.bin]\n");
        return -1;
    }

    printf("Nortel Millennium COINVL Table (Table 50) Dump\n\n");

    pcoinvl_table = (dlog_mt_coin_val_table_t *)calloc(1, sizeof(dlog_mt_coin_val_table_t));

    if (pcoinvl_table == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_coin_val_table_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(pcoinvl_table);
        return -ENOENT;
    }

    if (fread(pcoinvl_table, sizeof(dlog_mt_coin_val_table_t), 1, instream) != 1) {
        printf("Error reading COINVL table.\n");
        free(pcoinvl_table);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    printf("+---------------------------------------------+\n" \
           "|  # | Coin Type         | Val | Vol | Params |\n" \
           "+----+-------------------+-----+-----+--------+");

    for (coin_index = 0; coin_index < COIN_TYPES_MAX; coin_index++) {
        printf("\n| %2d | %s | %3d | %3d |     %2d |",
               coin_index + 1,
               str_coin_name[coin_index],
               pcoinvl_table->coin_value[coin_index],
               pcoinvl_table->coin_volume[coin_index],
               pcoinvl_table->coin_param[coin_index]);
    }

    printf("\n+---------------------------------------------+\n");

    printf("|           Cash Box Volume:   %5d          |\n",    pcoinvl_table->cash_box_volume);
    printf("|             Escrow Volume:   %5d          |\n",    pcoinvl_table->escrow_volume);
    printf("| Cash Box Volume Threshold:   %5d          |\n",    pcoinvl_table->cash_box_volume_threshold);
    printf("|  Cash Box Value Threshold: $%6.2f          |\n",   (float)(pcoinvl_table->cash_box_value_threshold) / 100.0);
    printf("|   Escrow Volume Threshold:   %5d          |\n",    pcoinvl_table->escrow_volume_threshold);
    printf("|    Escrow Value Threshold: $%6.2f          |\n", (float)(pcoinvl_table->escrow_value_threshold) / 100.0);
    printf("+---------------------------------------------+\n");

    pcoinvl_table->coin_param[cdn_nickel]  = 0x03;
    pcoinvl_table->coin_param[cdn_nickel2] = 0x03;
    pcoinvl_table->coin_param[cdn_dime]    = 0x03;
    pcoinvl_table->coin_param[cdn_quarter] = 0x03;
    pcoinvl_table->coin_param[cdn_dollar]  = 0x03;

    /* Add coin params for US Dollar */
    pcoinvl_table->coin_param[us_dollar]  = 0x03;
    pcoinvl_table->coin_value[us_dollar]  = 100;
    pcoinvl_table->coin_volume[us_dollar] = 40;

    /* Add coin params for Canadian Steel nickel, dime, quarter */
    pcoinvl_table->coin_param[cdn_steel_nickel]  = 0x03;
    pcoinvl_table->coin_value[cdn_steel_nickel]  = 5;
    pcoinvl_table->coin_volume[cdn_steel_nickel] = 20;

    pcoinvl_table->coin_param[cdn_steel_dime]  = 0x03;
    pcoinvl_table->coin_value[cdn_steel_dime]  = 10;
    pcoinvl_table->coin_volume[cdn_steel_dime] = 10;

    pcoinvl_table->coin_param[cdn_steel_quarter]  = 0x03;
    pcoinvl_table->coin_value[cdn_steel_quarter]  = 25;
    pcoinvl_table->coin_volume[cdn_steel_quarter] = 25;

    /* Add coin params for New Canadian Dollar coin */
    pcoinvl_table->coin_param[cdn_dollar2]  = 0x03;
    pcoinvl_table->coin_value[cdn_dollar2]  = 100;
    pcoinvl_table->coin_volume[cdn_dollar2] = 40;

    if (argc > 2) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            free(pcoinvl_table);
            return -ENOENT;
        }
    }

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(pcoinvl_table, sizeof(dlog_mt_coin_val_table_t), 1, ostream) != 1) {
            printf("Error writing output file %s\n", argv[2]);
            ret = -EIO;
        }
        fclose(ostream);
    }

    if (pcoinvl_table != NULL) {
        free(pcoinvl_table);
    }

    return ret;
}
