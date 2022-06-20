/*
 * Utility dump tables from Nortel Millennium Payphone firmware ROMs
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2022, Howard M. Harte
 *
 * Example:
 *
 * MTR 1.7 ROMs:
 * mm_table_cutter NT_NAA1S05.bin 12502 91
 * mm_table_cutter NT_FW_1.7_06CAA17_STM27C2001_32DIP.BIN 12453 91
 * (Card-only): mm_table_cutter NT_FW_M06CBBX1_M27C2001_DIP32.BIN 12502 91
 * (Coin-only w/display): mm_table_cutter NAD4K02.bin 12453 91
 * (Coin-only no display): mm_table_cutter NAD4S02.bin 12453 91
 * (Desk) mm_table_cutter NAJ2S05.bin 12547 91
 *
 * MTR 1.9 ROMs:
 * mm_table_cutter NT_Millennium_Demo_SST39SF020A-70-4C-PHE.bin 14499 107
 * mm_table_cutter NT_NBA1F02.bin 13270 107
 *
 * MTR 1.20 ROM:
 * mm_table_cutter NT_FW_1.20_NPA1S01_V1.0_STM27C2001_32DIP.bin 6090 151
 * mm_table_cutter NT_FW_1.20_NPE1S01_V1.0_M27C2001_DIP32_Coin_Basic.bin 6090 151
 *
 * MTR 2.12 ROM:
 * mm_table_cutter NT_FW_2.12_NQA1X01V1.3_U2_Rev2_CPC_STM29F040B_32PLCC.bin 10524 152
 *
 * International
 * mm_table_cutter PBAXS05.BIN 13121 93
 *
 * Determining table data structure offset:
 *
 * 1. Use a hex dump program like WinHex or hexdump to find the starting
 *    address of an easily-recognizable table, such as the visual prompts
 *    table.  Search for this address in the ROM.  Use that as an offset
 *    to remove the beginning of the ROM up until this point, for example:
 *
 *    dd if=NAA1S05.bin of=table_list.bin bs=1 count=1030 skip=12502
 *
 * 2. Use hexdump to list the tables, and then remove less of the
 *    ROM with 'dd' until the output with table
 *    1.  Each table entry is 10 bytes.
 *
 *    hexdump -v -e '/10 "%010_ad  |"'  -e '10/1 "%02x "' -e '"\n"' table_list.bin
 *
 * 3. Determine how many valid table entries are contained within the ROM.
 *    For example:
 *
 * 0000000630  |54 00 06 00 64 a0 01 00 05 2e
 * 0000000640  |5a 47 33 03 c6 bc 03 00 9f 16
 * 0000000650  |5b 47 33 03 fc bf 03 00 a2 16
 * 0000000660  |42 6a 33 60 ff 00 00 00 00 ef
 *
 * This ROM has 5b entries, converted to decimal is 91.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "./mm_manager.h"

#if defined(_MSC_VER)
# include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else  /* if defined(_MSC_VER) */
# include <unistd.h>
#endif /* if defined(_MSC_VER) */

#define TABLE_MAX       152
#define TABLE_LEN_MASK  0x1FFF      /* Maximum table length 8K */

#pragma pack(push)
#pragma pack(1)         /* Pack data structures for communication with terminal. */

typedef struct mt_table_entry {
    uint8_t id;
    uint8_t pad1;
    uint16_t len;
    uint8_t pad[4];
    uint16_t rom_addr;
} mt_table_entry_t;

#pragma pack(pop)

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   index;
    char  outfile[21];
    int   ret = 0;
    ssize_t rom_pos;
    uint8_t *ptable_buf;
    uint8_t last_table = TABLE_MAX;

    mt_table_entry_t *ptable_entry;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_table_cutter firmware.bin <offset> [last_table]\n");
        return -EINVAL;
    }

    printf("Nortel Millennium Table Cutter\n\n");

    ptable_entry = (mt_table_entry_t *)calloc(1, sizeof(mt_table_entry_t));

    if (ptable_entry == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(mt_table_entry_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(ptable_entry);
        return -ENOENT;
    }

    printf("Offset: 0x%05x (%d)\n", atoi(argv[2]), atoi(argv[2]));

    if (argc > 3) {
        last_table = atoi(argv[3]);
    }

    printf("Last table: %d\n", last_table);
    if (fseek(instream, atoi(argv[2]), SEEK_SET) != 0) {
        fprintf(stderr, "Error: fseek() to ROM table entry point failed.\n");
        goto done;
    }

    rom_pos = ftell(instream);

    if (rom_pos < 0) {
        fprintf(stderr, "Error: ftell() failed.\n");
        goto done;
    }

    printf("+---------------------------------------------------------------------------+\n" \
           "| Idx        | Table                       | Pad1 | Length | Address | Dir  |\n" \
           "+------------+-----------------------------+------+--------+---------+------+\n");

    for (index = 1; index <= TABLE_MAX; index++) {
        if (fseek(instream, (long)rom_pos, SEEK_SET) != 0) {
            fprintf(stderr, "Error: fseek() failed.\n");
            goto done;
        }

        if (fread(ptable_entry, sizeof(mt_table_entry_t), 1, instream) != 1) {
            fprintf(stderr, "Error reading table entry %d.\n", index);
            ret = -EIO;
            goto done;
        }

        rom_pos = ftell(instream);
        if (rom_pos < 0) {
            fprintf(stderr, "Error: ftell() failed.\n");
            goto done;
        }

        if (argv[3] != NULL && argv[4] == NULL) {
            printf("| 0x%02x (%3d) | %27s | 0x%02x |  %4d  |  0x%04x | %s | ",
                ptable_entry->id, ptable_entry->id,
                table_to_string(ptable_entry->id),
                ptable_entry->pad1,
                ptable_entry->len,
                ptable_entry->rom_addr,
                ptable_entry->rom_addr != 0 ? "M->T" : "    ");

            if (ptable_entry->rom_addr != 0) {
                if (fseek(instream, ptable_entry->rom_addr, SEEK_SET) != 0) {
                    fprintf(stderr, "Error: fseek() failed.\n");
                    goto done;
                }

                snprintf(outfile, sizeof(outfile), "cut_table_%02x.bin", ptable_entry->id);
                printf("Dumping %d bytes at %04x\n", ptable_entry->len, ptable_entry->rom_addr);
                if ((ostream = fopen(outfile, "wb")) == NULL) {
                    printf("Error opening output file %s for write.\n", outfile);
                    ret = -ENOENT;
                    goto done;
                }

                ptable_entry->len &= TABLE_LEN_MASK;
                ptable_buf = (uint8_t *)calloc(1, ptable_entry->len);

                if (ptable_buf == NULL) {
                    printf("Failed to allocate %d bytes.\n", ptable_entry->len);
                    ret = -ENOMEM;
                    goto done;
                }
                if (fread(ptable_buf, ptable_entry->len, 1, instream) != 1) {
                    printf("Error reading table %d from ROM.\n", index);
                    free(ptable_buf);
                    ret =  -EIO;
                    goto done;
                }

                if (fwrite(ptable_buf, ptable_entry->len, 1, ostream) != 1) {
                    printf("Error writing ROM table %d.\n", index);
                    free(ptable_buf);
                    ret = -EIO;
                    goto done;
                }
                free(ptable_buf);
            } else {
                printf("\n");
            }
        } else {
            printf("0x%02x,%s,0x%02x,%d\n",
                ptable_entry->id,
                table_to_string(ptable_entry->id),
                ptable_entry->pad1,
                ptable_entry->len);
        }
        if (ptable_entry-> id == last_table) break;
    }

    printf("+---------------------------------------------------------------------------+\n");

done:
    fclose(instream);
    if (ostream != NULL) {
        fclose(ostream);
    }

    if (ptable_entry != NULL) {
        free(ptable_entry);
    }

    return ret;
}
