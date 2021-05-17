/*
 * Code to dump LCD table from Nortel Millennium Payphone
 * Tables 136-138 (0x88-0x8a)
 * 
 * www.github.com/hharte/mm_manager
 * 
 * (c) 2020, Howard M. Harte
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

#include<stdio.h>

char *str_flags[4] = {
    " L ",  /* 0 - Local */
    " ? ",  /* 1 - ??? Inter-LATA toll? */
    "$LD",  /* 2 - Intra-LATA toll */
    " - "   /* 3 - Invalid NPA/NXX */
};

int main(int argc, char *argv[])
{
    FILE *instream;
    unsigned char c;
    int nxx;
    unsigned char npa_char[2];
    unsigned char check_digit;
    int npa;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_lcd mm_table_88.bin\n");
        return (-1);
    }

    instream = fopen(argv[1], "rb");

    printf("Nortel Millennium Double-Compressed LCD Table (Tables 136-138) Dump\n\n");
    npa_char[0] = fgetc(instream);
    if (npa_char[0] < 0x20 || npa_char[0] > 0x99) {
        printf("Invalid NPA!\n");
        fclose(instream);
        return -1;
    }

    npa_char[1] = fgetc(instream);
    check_digit = (npa_char[1] & 0x0f);

    if (check_digit != 0xe) {
        printf("Invalid check digit, expected 0xe!\n");
        fclose(instream);
        return -1;
    }

    npa  = ((npa_char[0] & 0xf0) >> 4) * 100;
    npa +=  (npa_char[0] & 0x0f) * 10;
    npa += ((npa_char[1] & 0xf0) >> 4);

    for (nxx = 200; nxx <= 999; nxx++) {
        unsigned char flag_mask = 0;
        unsigned char flag_shift;
        unsigned char flags;
        unsigned char twobit_group;

        if (nxx % 200 == 0) {
            printf("\n+---------------------------------------------------------------------+\n" \
                     "| NPA-NXX |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |\n" \
                     "+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+");
        }

        if (nxx % 10 == 0) {
            printf("\n| %03d-%02dx |", npa, nxx / 10);
        }

        twobit_group = nxx % 4;
        if (twobit_group == 0) {
            c = fgetc(instream);
            if (feof(instream)) break;
        }

        flag_mask = 0xc0 >> (twobit_group * 2);
        flag_shift = 6 - (twobit_group * 2);
        
        flags = (c & flag_mask) >> flag_shift;

        if (argc < 3) {
            printf(" %s |", str_flags[flags]);
        } else {
            printf("  %d  |", flags);
        }
    }       

    printf("\n+---------------------------------------------------------------------+\n");
    return 0;
}
