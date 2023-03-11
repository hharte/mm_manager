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
    size_t i, card_len, check_pos = 0;
    int sum = 0;

    if (argc <= 1) {
        printf("Usage:\n" \
            "\tmm_luhn <n-digit card number>\n\n" \
            "If all digits are specified, the card number is checked.\n" \
            "If one digit is replaced with a '?', the check digit in that\n" \
            "position will be generated.\n\n" \
            "Examples:\n" \
            "\tmm_luhn 4012888888881881 - check 16-digit card number.\n" \
            "\tmm_luhn 401288888888188? - generate check digit.\n");
        return -1;
    }

    card_len = strlen(argv[1]);
    for (i = 0; i < card_len; i++) {
        int single_digit = !(i & 1);
        int digit_val = argv[1][card_len - i - 1];
        int digit = digit_val - '0';

        if (digit_val == '?') {
            luhn_check = 0;
            check_pos = card_len - i - 1;
            digit = 0;
        }

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
        if (check_digit == 10) check_digit = 0;

        if (check_pos % 2 == 0) check_digit ++;
        argv[1][check_pos] = '0' + check_digit;
        printf("Card number: %s\n", argv[1]);
//        printf("n=%zu, sum = %d, pos = %zu, check digit = %d\n", i, sum, check_pos, check_digit);
    }
    else {
        if (sum % 10) {
            printf("Invalid check digit: Final sum = %d, remainder=%d, check result=%d\n", sum, sum % 10, (sum % 10));
        }
        else {
            printf("Card number %s: Ok\n", argv[1]);
        }
    }
    return (0);
}
