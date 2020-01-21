/*
 * This is a "Manager" for the Nortel Millennium payhone.
 * 
 * It can provision a Nortel Millennium payphone with Rev 1.0 or 1.3
 * Control PCP.  CDRs, Alarms, and Maintenance Reports can also be
 * retieved.
 * 
 * www.github.com/hharte/mm_manager
 *  
 * (c) 2020, Howard M. Harte
 */

#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>  /* String function definitions */

#define POLY 0xa001     /* Polynomial to use for CRC-16 calculation */

/* Calculate CRC-16 checksum using 0xA001 polynomial. */
unsigned crc16(unsigned crc, uint8_t *buf, size_t len)
{
    while (len--) {
        crc ^= *buf++;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    }
    return crc;
}

void dump_hex(uint8_t *data, int len)
{
    uint8_t ascii[32];
    uint8_t *pascii = ascii;
    int i;

    printf("\n");
    if (len > 0) {
        printf("\tData: ");

        for (i = 0; i < len; i++) {
            if (i % 16 == 0) {
                if (i > 0) {
                    *pascii++ = '\0';
                    printf("%s", ascii);
                }
                printf("\n\t%03d: ", i);
                pascii = ascii;
            }
            printf("%02x, ", data[i]);
            if ((data[i] >= 0x20) && (data[i] < 0x7F)) {
                *pascii++ = data[i];
            } else {
                *pascii++ = '.';    
            }
        
        }
        *pascii++ = '\0';
        if (strlen((char *)ascii) > 0) {
            for (i = 0; i < 16 - strlen((char *)ascii); i++) {
                printf("    ");
            }
            printf("%s", ascii);
        }
    }           
    printf("\n");
}
