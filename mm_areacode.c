/*
 * Code to dump area codes list from Nortel Millennium
 * payphone table 0x96
 *
 * www.github.com/hharte/mm_manager
 *
 * (c) 2020-2022, Howard M. Harte
 *
 * Table 0x96 is an array of 400 bytes, where each nibble corresponds
 * to one of the area codes from 200-999.
 *
 * Each nibble contains a value representing the type of area code
 * as follows:
 * 0 - Invalid (for example, 211, 311, 800, 900)
 * 2 - Unassigned at this time.
 * 4 - US / Canada Area Code.
 * 6 - International Area Code (for example, the Bahamas.)
 *
 */

#include<stdio.h>

char *str_flags[4] = {
    "---",  /* 0 - Invalid NPA */
    "   ",  /* 1 - Unassigned */
    "USA",  /* 2 - Local, Intra-LATA, Inter-LATA, depending on LCD Table */
    "For"
};

int main(int argc, char *argv[])
{
    FILE *instream;
    int areacode = 200;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_areacode mm_table_96.bin\n");
        return (-1);
    }

    instream = fopen(argv[1], "rb");

    printf("Nortel Millennium NPA (Table 0x96) Dump");
    while(1) {
        unsigned char c;
        unsigned char flags0, flags1;

        c = (unsigned char)fgetc(instream);
        if (feof(instream)) break;

        if (areacode % 200 == 0) {
            printf("\n+-----------------------------------------------------------------+\n" \
                     "| NPA |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |\n" \
                     "+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+");
        }

        if (areacode % 10 == 0) {
            printf("\n| %02dx |", areacode / 10);
        }

        flags0 = (c & 0x70) >> 4;
        flags1 = c & 0x07;

        if (argc < 3) {
            printf(" %s | %s |", str_flags[flags0 >> 1], str_flags[flags1 >> 1]);
        } else {
            printf("  %x  |  %x  |", flags0, flags1);
        }
        areacode += 2;
    }

    printf("\n+-----------------------------------------------------------------+\n");
    return 0;
}
