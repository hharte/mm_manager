/*
 * Code to dump LCD table from Nortel Millennium Payphone
 * Tables 136-138 (0x88-0x8a)
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 *
 * The LCD Table is an array of 202 bytes.  The first two bytes contain
 * the NPA followed by 'e', for example, NPA 408 would be represented as
 * 0x40 0x8e.
 *
 * The remaining 200 bytes represent two bits for each NXX in the range
 * of 200-999.
 *
 * These two bits encode a value as follows:
 * 0 - Local Rate
 * 1 - ?
 * 2 - Intra-LATA toll NXX
 * 3 - Invalid (ie, the N11, the NPA itself.
 *
 * See: https://nationalnanpa.com/reports/reports_cocodes_assign.html
 *
 * All NXX in the entire USA: wget https://nationalnanpa.com/nanp1/allutlzd.zip
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "./mm_manager.h"

#define LCD_TABLE_LEN                   (sizeof(dlog_mt_lcd_table_t))
#define COMPRESSED_LCD_TABLE_LEN        (sizeof(dlog_mt_compressed_lcd_table_t))
#define DOUBLE_COMPRESSED_LCD_TABLE_LEN (sizeof(dlog_mt_npa_nxx_table_t))

const char *str_flags[4] = {
    " L ", /* 0 - Local */
    " ? ", /* 1 - ??? Inter-LATA toll? */
    "$LD", /* 2 - Intra-LATA toll */
    " - "  /* 3 - Invalid NPA/NXX */
};

const char *str_flags_mtr1[16] = {
    " L ", /* 0 - Local */
    " ? ", /* 1 - LMS */
    "$LD", /* 2 - Intra-LATA toll */
    " - ", /* 3 - Invalid NPA/NXX */
    "$$$", /* 4 - Inter-LATA toll */
    "  5",
    "  6",
    "  7",
    "  8",
    "  9",
    " 10",
    " 11",
    " 12",
    " 13",
    " 14",
    " 15"
};

int main(int argc, char *argv[]) {
    FILE *instream;
    dlog_mt_lcd_table_t *lcd_table;
    uint8_t* load_buffer;
    uint8_t c = 0;
    int     nxx;
    uint8_t npa_char[2] = { 0 };
    uint8_t check_digit;
    int     npa;
    size_t  size;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_lcd mm_table_88.bin\n");
        return -1;
    }

    printf("Nortel Millennium LCD Table Dump\n\n");

    lcd_table = (dlog_mt_lcd_table_t *)calloc(1, sizeof(dlog_mt_lcd_table_t));

    if (lcd_table == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_lcd_table_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(lcd_table);
        return -ENOENT;
    }

    fseek(instream, 0, SEEK_END);
    size = ftell(instream);
    fseek(instream, 0, SEEK_SET);

    switch (size + 1) {
        case LCD_TABLE_LEN:
            printf("Uncompressed LCD table.\n");
            break;
        case COMPRESSED_LCD_TABLE_LEN:
            printf("Compressed LCD table.\n");
            break;
        case DOUBLE_COMPRESSED_LCD_TABLE_LEN:
            printf("Double-compressed LCD table.\n");
            break;
        default:
            printf("Invalid LCD table.\n");
            free(lcd_table);
            fclose(instream);
            return -EIO;
    }

    load_buffer = ((uint8_t*)lcd_table) + 1;
    if (fread(load_buffer, size, 1, instream) != 1) {
        printf("Error reading LCD table.\n");
        free(lcd_table);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    npa_char[0] = lcd_table->term_npa_nxx[0];

    if ((npa_char[0] < 0x20) || (npa_char[0] > 0x99)) {
        printf("Invalid NPA, must be in the range of 200-999.\n");
        free(lcd_table);
        return -1;
    }

    if (size == LCD_TABLE_LEN - 1) {
        npa_char[1] = lcd_table->called_npa[1];
        check_digit = (npa_char[1] & 0x0f);
    } else {
        npa_char[1] = ((dlog_mt_npa_nxx_table_t *)lcd_table)->npa[1];
        check_digit = (npa_char[1] & 0x0f);
    }

    if (check_digit != 0xe) {
        printf("Invalid check digit 0x%x, expected 0xe!\n", check_digit);
        free(lcd_table);
        return -1;
    }

    npa  = ((npa_char[0] & 0xf0) >> 4) * 100;
    npa +=  (npa_char[0] & 0x0f) * 10;
    npa += ((npa_char[1] & 0xf0) >> 4);

    for (nxx = 200; nxx <= 999; nxx++) {
        uint8_t flag_mask = 0;
        uint8_t flag_shift;
        uint8_t flags;
        uint8_t twobit_group;

        if (nxx % 200 == 0) {
            printf("\n+---------------------------------------------------------------------+\n" \
                   "| NPA-NXX |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |\n"   \
                   "+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+");
        }

        if (nxx % 10 == 0) {
            printf("\n| %03d-%02dx |", npa, nxx / 10);
        }

        switch(size + 1) {
        case LCD_TABLE_LEN:
            c = ((dlog_mt_lcd_table_t *)lcd_table)->lcd[nxx - 200];
            flag_shift = 0;
            flag_mask = 0xFF;
            break;
        case COMPRESSED_LCD_TABLE_LEN:
            if (nxx % 2 == 0) {
                c = ((dlog_mt_compressed_lcd_table_t *)lcd_table)->lcd[(nxx - 200) / 2];
                flag_mask  = 0xF0;
                flag_shift = 4;
            } else {
                flag_mask = 0x0F;
                flag_shift = 0;
            }
            break;
        case DOUBLE_COMPRESSED_LCD_TABLE_LEN:
            twobit_group = nxx % 4;

            if (twobit_group == 0) {
                c = ((dlog_mt_npa_nxx_table_t *)lcd_table)->lcd[(nxx - 200) / 4];
            }

            flag_mask  = 0xc0 >> (twobit_group * 2);
            flag_shift = 6 - (twobit_group * 2);
            break;
        default:
            printf("Invalid LCD table!\n");
            if (lcd_table != NULL) {
                free(lcd_table);
            }
            return -1;
        }

        flags = (c & flag_mask) >> flag_shift;

        if (argc < 3) {
            if (size + 1 == DOUBLE_COMPRESSED_LCD_TABLE_LEN) {
                printf(" %s |", str_flags[flags & 0x03]);
            } else {
                printf(" %s |", str_flags_mtr1[flags & 0x0f]);
            }

        } else {
            printf("  %d  |", flags);
        }
    }

    if (lcd_table != NULL) {
        free(lcd_table);
    }

    printf("\n+---------------------------------------------------------------------+\n");
    return 0;
}
