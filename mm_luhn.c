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
    int luhn_check = 1;
    size_t i = 0;
    int sum = 0;

    if (argc <= 1) {
        printf("Usage:\n" \
            "\tmm_luhn <n-digit card number> - to verify the check digit.\n" \
            "\tmm_luhn <n-digit card number> -g - to generate the check digit.\n");
        return -1;
    }

    if (argc > 2) {
        luhn_check = 0;
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
        uint8_t check_digit = 10 - (sum % 10);
        if (i % 2 == 0) check_digit /= 2;
        printf("n=%zu, %d\n", i, check_digit);
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
