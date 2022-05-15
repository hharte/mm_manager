/*
 * Nortel Millennium Dialog to .pcap Conversion Utility
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2019-2022, Howard M. Harte
 *
 * This utility converts dialog transcripts from mm_manager into
 * packet capture files (.pcap) suitable for analysis in Wireshark.
 *
 */

#include <errno.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>  /* String function definitions */
#ifndef _WIN32
#include <libgen.h>
#endif /* ifndef _WIN32 */

#include "./mm_manager.h"

#define L2_STATE_SEARCH_FOR_START   1
#define L2_STATE_GET_FLAGS          2
#define L2_STATE_GET_LENGTH         3
#define L2_STATE_ACCUMULATE_DATA    4
#define L2_STATE_GET_CRC0           5
#define L2_STATE_GET_CRC1           6
#define L2_STATE_SEARCH_FOR_STOP    7

volatile int inject_comm_error = 0;

typedef struct mm_parser {
    uint8_t      l2_state;
    mm_packet_t  pkt;
    pkt_status_t status;
} mm_parser_t;

uint8_t mm_parse_byte(mm_parser_t* context, uint8_t databyte, uint32_t line);
uint8_t mm_parser_reset(mm_parser_t* context);

int  main(int argc, char *argv[]) {
    FILE *instream   = NULL;
    FILE *pcapstream = NULL;

    char buf[100];  /* buffer to hold a line of data from the input byte stream */

    int count;
    int line = 0;
    uint32_t start_time, stop_time;
    uint32_t ts_sec, ts_usec;
    uint32_t packets_processed = 0;
    char direction;
    uint8_t pkt_direction;

    unsigned int databyte;
    int status = 0;

    mm_parser_t rxparser;
    mm_parser_t txparser;

    mm_parser_t* parser = NULL;

    printf("Nortel Millennium Dialog to .pcap Conversion Utility\n\n");

    if (argc <= 2) {
        printf("Usage: %s <filename.dlog> <filename.pcap>\n", basename(argv[0]));
        status = -EINVAL;
        goto done;
    }

    if (!(instream = fopen(argv[1], "r"))) {
        fprintf(stderr, "%s: Can't read '%s': %s\n",
            basename(argv[0]), argv[1], strerror(errno));
        status = -ENOENT;
        goto done;
    }

    if (mm_create_pcap(argv[2], &pcapstream) != 0) {
        fprintf(stderr, "%s: Can't write '%s': %s\n",
            basename(argv[0]), argv[2], strerror(errno));
        status = -ENOENT;
        goto done;
    }

    mm_parser_reset(&rxparser);
    mm_parser_reset(&txparser);

    while (!feof(instream)) {
        line++;
        fgets(buf, 80, instream);

        /* TX/RX in file are from Terminal's perspective. */
        count = sscanf(buf, "%u-%u UART: %cX: %2x", &start_time, &stop_time, &direction, &databyte);

        if (count == 4) {
//            printf("Count=%d, Character start %d, stop %d, dir=%c, data=0x%02x\n", count, start_time, stop_time, direction, databyte);
        } else {
            count = sscanf(buf, "UART: %cX: %2x", &direction, &databyte);
            if (count != 2) {
                fprintf(stderr, "Error parsing input stream, line=%d", line);
                goto done;
            }
            start_time = stop_time = 0;
//            printf("Count=%d, dir=%c, data=0x%02x\n", count, direction, databyte);
        }

        if (direction == 'R') {
            parser = &rxparser;
        } else {
            parser = &txparser;
        }

        status = mm_parse_byte(parser, databyte, line);

        pkt_direction = (parser == &txparser) ? TX : RX;
        if (status == 1) {
            ts_sec = stop_time / 20000;
            ts_usec = (stop_time % 20000) * 100;

            packets_processed++;
            mm_add_pcap_rec(pcapstream, pkt_direction, &parser->pkt, ts_sec, ts_usec);
//            print_mm_packet(pkt_direction, &parser->pkt);
            mm_parser_reset(parser);
        }
    }

    printf("Processed %u packets\n", packets_processed);

done:
    if (instream != NULL) {
        fclose(instream);
    }

    mm_close_pcap(pcapstream);

    return status;
}

/*
 * Byte at a time parser for the Nortel Millennium Terminal.
 *
 * Packets are formatted as follows:
 * +------+-------+--------+-----------+--------+-----+
 * |START | FLAGS | LENGTH | DATA .... | CRC-16 | END |
 * +------+-------+--------+-----------+--------+-----+
 */
uint8_t mm_parse_byte(mm_parser_t* context, uint8_t databyte, uint32_t line) {
    uint8_t status = 0;

    switch (context->l2_state) {
    case L2_STATE_SEARCH_FOR_START:
        if (databyte == START_BYTE) {
            context->l2_state = L2_STATE_GET_FLAGS;
            context->pkt.payload_len = 0;
            context->pkt.hdr.start = databyte;
        }
        break;
    case L2_STATE_GET_FLAGS:
        context->l2_state = L2_STATE_GET_LENGTH;
        context->pkt.hdr.flags = databyte;
        break;
    case L2_STATE_GET_LENGTH:
        context->pkt.hdr.pktlen = databyte;

        if (context->pkt.hdr.pktlen > 5) {
            context->l2_state = L2_STATE_ACCUMULATE_DATA;
        }
        else {
            context->l2_state = L2_STATE_GET_CRC0;
        }
        break;
    case L2_STATE_ACCUMULATE_DATA:
        context->pkt.payload[context->pkt.payload_len] = databyte;
        context->pkt.payload_len++;

        if (context->pkt.payload_len == context->pkt.hdr.pktlen - 5) {
            context->l2_state = L2_STATE_GET_CRC0;
        }
        break;
    case L2_STATE_GET_CRC0:
        context->l2_state = L2_STATE_GET_CRC1;
        context->pkt.trailer.crc = databyte;
        break;
    case L2_STATE_GET_CRC1:
        context->l2_state = L2_STATE_SEARCH_FOR_STOP;
        context->pkt.trailer.crc |= databyte << 8;
        context->pkt.calculated_crc = crc16(0, &context->pkt.hdr.start, (size_t)context->pkt.payload_len + 3);

        if (context->pkt.trailer.crc != context->pkt.calculated_crc) {
            printf("%s: CRC Error in line %u!\n", __FUNCTION__, line);
            context->status |= PKT_ERROR_CRC;
        }
        break;
    case L2_STATE_SEARCH_FOR_STOP:
        if (databyte == STOP_BYTE) {
            context->l2_state = L2_STATE_SEARCH_FOR_START;
        }
        else {
            printf("%s: Framing Error in line %u!\n", __FUNCTION__, line);
            context->status |= PKT_ERROR_FRAMING;
        }
        context->pkt.trailer.end = databyte;
        status = 1;
        break;
    }

    /* Copy the packet trailer (CRC-16, STOP) immediately following the data */
    memcpy(&(context->pkt.payload[context->pkt.payload_len]), &context->pkt.trailer, sizeof(context->pkt.trailer));
    return status;
}

uint8_t mm_parser_reset(mm_parser_t* context) {
    context->l2_state = L2_STATE_SEARCH_FOR_START;
    context->status = 0;
    memset(&context->pkt, 0, sizeof(mm_packet_t));

    return 0;
}
