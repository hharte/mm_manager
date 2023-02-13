/*
 * Utility to generate / check card Luhn check digit.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2023, Howard M. Harte
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    int luhn_check = -1;
    int i = 0;
    int sum = 0;

    if (argc > 1) {
        switch (strlen(argv[1])) {
        case 16:
            luhn_check = 1;
            break;
        case 15:
            luhn_check = 0;
            break;
        default:
            printf("Usage:\n" \
                "\tmm_luhn <16-digit card number> - to verify the check digit.\n" \
                "\tmm_luhn <15-digit card number> - to generate the check digit.\n");
            return -1;
            break;
        }
    }

    if ((luhn_check == -1) || (argc <= 1)) {
        printf("Usage:\n" \
            "\tmm_luhn <16-digit card number> - to verify the check digit.\n" \
            "\tmm_luhn <15-digit card number> - to generate the check digit.\n");
        return -1;
    }

    printf("%s Luhn for card number %s: ", luhn_check ? "Checking" : "Generating", argv[1]);

    for (i = 0; i < strlen(argv[1]); i++) {
        int single_digit = (i & 1);
        int digit = argv[1][i] - '0';

        if (single_digit) {
            sum += digit;
        }
        else {
            digit *= 2;
            if (digit <= 9) {
                sum += digit;
            }
            else {
                sum += digit / 10;
                sum += digit % 10;
            }
        }
    }

    if (luhn_check == 0) {
        printf("%d\n", 10 - (sum % 10));
    }
    else {
        if (sum % 10) {
            printf("Invalid check digit: Final sum = %d, remainder=%d, check result=%d\n", sum, sum % 10, (sum % 10));
        }
        else {
            printf("Ok\n");
        }
    }
    return (0);
}
