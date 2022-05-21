/*
 * This is a "Manager" for the Nortel Millennium payhone.
 *
 * It can provision a Nortel Millennium payphone with Rev 1.0 or 1.3
 * Control PCP.  CDRs, Alarms, and Maintenance Reports can also be
 * retieved.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 */

#include <stdio.h>  /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h> /* String function definitions */
#include <time.h>
#ifdef _WIN32
# include <windows.h>
#else  /* ifdef _WIN32 */
# include <termios.h> /* POSIX terminal control definitions */
# include <unistd.h>
#endif /* _WIN32 */
#include "./mm_manager.h"
#include "./mm_serial.h"
#include "./mm_udp.h"

extern volatile int inject_comm_error;

#define L2_STATE_SEARCH_FOR_START   1
#define L2_STATE_GET_FLAGS          2
#define L2_STATE_GET_LENGTH         3
#define L2_STATE_ACCUMULATE_DATA    4
#define L2_STATE_GET_CRC0           5
#define L2_STATE_GET_CRC1           6
#define L2_STATE_SEARCH_FOR_STOP    7

/*
 * Receive a packet from Millennium Terminal.
 *
 * Packets are formatted as follows:
 * +------+-------+--------+-----------+--------+-----+
 * |START | FLAGS | LENGTH | DATA .... | CRC-16 | END |
 * +------+-------+--------+-----------+--------+-----+
 */
pkt_status_t receive_mm_packet(mm_context_t *context, mm_packet_t *pkt) {
    uint8_t pkt_received = 0;
    uint8_t databyte     = 0;
    uint8_t l2_state     = L2_STATE_SEARCH_FOR_START;
    pkt_status_t status  = PKT_SUCCESS;
    uint8_t timeout      = 0;

    if (context->connected == 0) {
        fprintf(stderr, "%s: Attempt to receive packet while disconnected, bailing.\n", __FUNCTION__);
        return PKT_ERROR_DISCONNECT;
    }

    while (pkt_received == 0) {
        int inject_error = 0;
        if (inject_comm_error == 1) {
            if (((context->error_inject_type == ERROR_INJECT_CRC_DLOG_RX) && (context->waiting_for_ack == 0)) ||
                ((context->error_inject_type == ERROR_INJECT_CRC_ACK_RX)  && (context->waiting_for_ack == 1))) {
                inject_error = (l2_state == L2_STATE_GET_CRC0) ? 1 : 0;
            }
        }
        if (inject_error == 1) {
            printf("Inject error type %d: Injecting error on READ now.\n", context->error_inject_type);
            inject_comm_error = 0;
        }
        while (read_serial(context->serial_context, &databyte, 1, inject_error) == 0) {
            if (context->connected == 0) {
                return PKT_ERROR_DISCONNECT;
            }
            putchar('.');
            fflush(stdout);
            timeout++;

            if (timeout > PKT_TIMEOUT_MAX) {
                printf("%s: Timeout waiting for packet error.\n", __FUNCTION__);
                status |= PKT_ERROR_TIMEOUT;
                return status;
            }
        }
        timeout = 0;

        switch (l2_state) {
            case L2_STATE_SEARCH_FOR_START:
                if (databyte == START_BYTE) {
                    l2_state         = L2_STATE_GET_FLAGS;
                    pkt->payload_len = 0;
                    pkt->hdr.start   = databyte;
                }
                break;
            case L2_STATE_GET_FLAGS:
                l2_state       = L2_STATE_GET_LENGTH;
                pkt->hdr.flags = databyte;
                break;
            case L2_STATE_GET_LENGTH:
                pkt->hdr.pktlen = databyte;

                if (pkt->hdr.pktlen > 5) {
                    l2_state = L2_STATE_ACCUMULATE_DATA;
                } else {
                    l2_state = L2_STATE_GET_CRC0;
                }
                break;
            case L2_STATE_ACCUMULATE_DATA:
                pkt->payload[pkt->payload_len] = databyte;
                pkt->payload_len++;

                if (pkt->payload_len == pkt->hdr.pktlen - 5) {
                    l2_state = L2_STATE_GET_CRC0;
                }
                break;
            case L2_STATE_GET_CRC0:
                l2_state         = L2_STATE_GET_CRC1;
                pkt->trailer.crc = databyte;
                break;
            case L2_STATE_GET_CRC1:
                l2_state            = L2_STATE_SEARCH_FOR_STOP;
                pkt->trailer.crc   |= databyte << 8;
                pkt->calculated_crc = crc16(0, &pkt->hdr.start, (size_t)pkt->payload_len + 3);

                if (pkt->trailer.crc != pkt->calculated_crc) {
                    printf("%s: CRC Error!\n", __FUNCTION__);
                    status |= PKT_ERROR_CRC;
                }
                break;
            case L2_STATE_SEARCH_FOR_STOP:
                if (databyte == STOP_BYTE) {
                    l2_state = L2_STATE_SEARCH_FOR_START;
                } else {
                    printf("%s: Framing Error!\n", __FUNCTION__);
                    status |= PKT_ERROR_FRAMING;
                }
                pkt->trailer.end = databyte;
                pkt_received     = 1;
                break;
        }
    }

    /* Copy the packet trailer (CRC-16, STOP) immediately following the data */
    memcpy(&(pkt->payload[pkt->payload_len]), &pkt->trailer, sizeof(pkt->trailer));

    mm_add_pcap_rec(context->pcapstream, RX, pkt, 0, 0);
    if (context->send_udp) {
        mm_udp_send_pkt(RX, pkt, 0, 0);
    }

    if (pkt->hdr.flags & FLAG_RETRY) {
        if (context->debuglevel > 0) print_mm_packet(RX, pkt);
        status |= PKT_ERROR_RETRY;
    }

    if (pkt->hdr.flags & FLAG_DISCONNECT) {
        if (context->debuglevel > 0) print_mm_packet(RX, pkt);
        printf("%s: Received disconnect status %s from terminal.\n", __FUNCTION__,
               pkt->hdr.flags & FLAG_STATUS ? "Failure" : "OK");
        context->tx_seq = 0;

        printf("%s: Hanging up modem.\n", __FUNCTION__);
        hangup_modem(context->serial_context);
        context->connected = 0;
        status |= PKT_ERROR_DISCONNECT;
    }

    if (context->debuglevel > 3) {
        printf("\nRaw Packet received: ");

        dump_hex(&pkt->hdr.start, (size_t)pkt->hdr.pktlen + 1);
    }

    return status;
}

/* Send manager packet to the terminal.
 *
 * If payload is NULL, an ACK packet will be sent.
 * If payload is not NULL, and the length is 0, a NULL packet will be sent.
 * If payload is not NULL, and length is > 0, then the terminal's phone number
 * will be prepended to the payload and sent.
 *
 * Returns PKT_SUCCESS on success, otherwise PKT_ERROR_ code flags.
 */
pkt_status_t send_mm_packet(mm_context_t* context, uint8_t* payload, size_t len, uint8_t flags) {
    mm_packet_t pkt = {{0}};
    pkt_status_t status = PKT_SUCCESS;
    int retries;

    for (retries = 0; retries < PKT_MAX_RETRIES; retries++) {
        /* Bail out if not connected. */
        if (context->connected != 1) {
            return PKT_ERROR_DISCONNECT;
        }

        if (context->debuglevel > 3) {
            if (payload != NULL) {
                printf("T<--M Sending packet: Terminal: %s, tx_seq=%d\n", context->terminal_id, context->tx_seq);
            }
            else {
                printf("T<--M Sending %s: rx_seq=%d\n", (flags & FLAG_ACK) ? "ACK" : "NACK", context->rx_seq);
            }
        }

        /* Insert Tx packet delay when using a modem, in 10ms increments. */
        if (context->use_modem == 1) {
#ifdef _WIN32
            Sleep(context->instsv.rx_packet_gap * 10);
#else  /* ifdef _WIN32 */
            struct timespec tim;
            tim.tv_sec = 0;
            tim.tv_nsec = context->instsv.rx_packet_gap * 10000000L;
            nanosleep(&tim, NULL);
#endif /* _WIN32 */
        }

        pkt.hdr.start = START_BYTE;

        if (payload != NULL) {
            /* Flags for regular TX packet use tx_seq. */
            pkt.hdr.flags = (context->tx_seq & FLAG_SEQUENCE);
            pkt.payload_len = (uint8_t)len + PKT_TABLE_ID_OFFSET; /* add room for the phone number. */

            for (int i = 0; i < PKT_TABLE_ID_OFFSET; i++) {
                pkt.payload[i] = (context->terminal_id[i * 2] - '0') << 4;
                pkt.payload[i] |= (context->terminal_id[i * 2 + 1] - '0');
            }

            if (len > 0) {
                memcpy(&pkt.payload[PKT_TABLE_ID_OFFSET], payload, len);
            }
        } else {
            pkt.payload_len = 0;

            /* If payload is NULL, send an ACK packet instead, using rx_seq. */
            pkt.hdr.flags = flags | (context->rx_seq & FLAG_SEQUENCE);
        }

        if (flags & FLAG_RETRY) {
            pkt.hdr.flags |= FLAG_RETRY;
        }

        pkt.hdr.pktlen = pkt.payload_len + 5;
        pkt.trailer.crc = crc16(0, &pkt.hdr.start, (size_t)(pkt.hdr.pktlen) - 2);
        if (inject_comm_error == 1) {
            if (((context->error_inject_type == ERROR_INJECT_CRC_DLOG_TX) && (pkt.payload_len != 0)) ||
                ((context->error_inject_type == ERROR_INJECT_CRC_ACK_TX) && (pkt.payload_len == 0))) {
                printf("Injecting %s (Correct CRC=0x%02x).\n",
                    error_inject_type_to_str(context->error_inject_type),
                    pkt.trailer.crc);
                inject_comm_error = 0;
                pkt.trailer.crc = ~pkt.trailer.crc;
            }
        }
        pkt.trailer.end = STOP_BYTE;
        pkt.calculated_crc = pkt.trailer.crc;

        /* Copy the CRC and STOP_BYTE to be adjacent to the filled portion of the payload */
        memcpy(&(pkt.payload[pkt.payload_len]), &pkt.trailer.crc, 3);

        mm_add_pcap_rec(context->pcapstream, TX, &pkt, 0, 0);
        if (context->send_udp) {
            mm_udp_send_pkt(TX, &pkt, 0, 0);
        }

        if (context->debuglevel > 0) {
            print_mm_packet(TX, &pkt);
        }

        if (context->debuglevel > 3) {
            printf("\nRaw Packet transmitted: ");
            dump_hex(&pkt.hdr.start, (size_t)pkt.hdr.pktlen + 1);
        }

        write_serial(context->serial_context, &pkt, (size_t)pkt.hdr.pktlen + 1);
        drain_serial(context->serial_context);

        /* Don't wait for ACK if sending an ACK. */
        if (payload == NULL) {
            break;
        }

        status = wait_for_mm_ack(context);
        if (status == PKT_SUCCESS) {
            break;
        }

        printf("%s: Received NACK, retrying %d.\n", __FUNCTION__, retries);
    }

    if (retries == PKT_MAX_RETRIES) {
        printf("%s: Error: Gave up after %d retries.\n", __FUNCTION__, retries);
    }

    if (payload != NULL) {
        context->tx_seq++;
    } else {
        context->rx_seq++;
    }
    return status;
}

pkt_status_t send_mm_ack(mm_context_t *context, uint8_t flags) {
    return send_mm_packet(context, NULL, 0, flags);
}

pkt_status_t wait_for_mm_ack(mm_context_t *context) {
    mm_packet_t pkt;
    pkt_status_t status = PKT_SUCCESS;

    if (context->connected == 0) {
        return PKT_ERROR_DISCONNECT;
    }

    context->waiting_for_ack = 1;
    status = receive_mm_packet(context, &pkt);
    context->waiting_for_ack = 0;
    if ((status == PKT_SUCCESS) || (status == PKT_ERROR_DISCONNECT)) {
        if (context->debuglevel > 2) print_mm_packet(RX, &pkt);

        if (pkt.payload_len == 0) {
            if (pkt.hdr.flags & FLAG_ACK) {
                return PKT_SUCCESS;
            } else {
                /* ACK flag is not set: NACK. */
                return PKT_ERROR_NACK;
            }
        }
        return PKT_SUCCESS;
    }
    fprintf(stderr, "%s: Error, did not receive an ACK packet, status=0x%02x\n", __FUNCTION__, status);
    return status;
}

int print_mm_packet(int direction, mm_packet_t *pkt) {
    int status = 0;

    /* Decode flags: bit 3 = Req/Ack, 2=Retry, 1:0=Sequence. */
    printf("\n%s %s: flags=%02x [ %s | %s | %s | %s | Seq:%d], len=%3d (datalen=%3d), crc=%04x.\n",
           (direction == RX) ? "T-->M" : "T<--M",
           (direction == RX) ? "RX" : "TX",
           pkt->hdr.flags,
           (pkt->hdr.flags & FLAG_DISCONNECT) ? "DIS" : "---",
           (pkt->hdr.flags & FLAG_STATUS) ? "ERR" : "OK ",
           (pkt->hdr.flags & FLAG_ACK) ? "ACK" : "REQ",
           (pkt->hdr.flags & FLAG_RETRY) ? "RETRY" : " --- ",
           (pkt->hdr.flags & FLAG_SEQUENCE),
           pkt->hdr.pktlen, pkt->payload_len, pkt->trailer.crc);

    if (pkt->trailer.crc != pkt->calculated_crc) {
        printf("\t*** CRC Error, calculated: %04x ***\n", pkt->calculated_crc);
        status = -1;
    }

    if (pkt->trailer.end != STOP_BYTE) {
        printf("\t*** Framing Error, expected STOP=0x03, got STOP=%02x ***\n", pkt->trailer.end);
        status = -1;
    }

    if (pkt->payload_len > 0) {
        dump_hex(pkt->payload, pkt->payload_len);
    }

    return status;
}

