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
#include <termios.h> /* POSIX terminal control definitions */
#include <unistd.h>

#include "mm_manager.h"

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
int receive_mm_packet(mm_context_t *context, mm_packet_t *pkt)
{
    char buf[100];  // buffer to hold line of data from UART protocol analyzer */
    uint8_t pktbuf[1024];
    uint8_t *pktbufp;
    uint8_t pkt_received = 0;
    char *bytep;
    unsigned int databyte;
    unsigned int bytecnt = 0;
    int bytes_processed;
    uint8_t l2_state = L2_STATE_SEARCH_FOR_START;;
    
    pktbufp = pktbuf;

    while(pkt_received == 0) {
        if (context->use_modem == 1) {
            while(read(context->fd, &databyte, 1) == 0) {
                putchar('.');
                fflush(stdout);
            }
        } else {
            if (feof(context->bytestream)) {
                break;
            }
            fgets(buf, 80, context->bytestream);

            /* Data that came from the Millennium Terminal. */
            if ((bytep = strstr(buf, "RX: ")) > 0) {
                if (bytep == NULL) return -1;
                sscanf(bytep, "RX: %x", &databyte);
            }
        }
        databyte &= 0xFF;

        if(context->logstream != NULL) {
            fprintf(context->logstream, "UART: RX: %02X\n", databyte);
            fflush(context->logstream);
        }
        
        switch (l2_state) {
            case L2_STATE_SEARCH_FOR_START:
                if (databyte == START_BYTE) {
                    l2_state = L2_STATE_GET_FLAGS;
                    pkt->payload_len = 0;
                    pkt->hdr.start = databyte;
                }
                break;
            case L2_STATE_GET_FLAGS:
                l2_state = L2_STATE_GET_LENGTH;
                pkt->hdr.flags = databyte;
                break;
            case L2_STATE_GET_LENGTH:
                pkt->hdr.pktlen = databyte;
                if(pkt->hdr.pktlen > 5) {
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
                l2_state = L2_STATE_GET_CRC1;
                pkt->trailer.crc = databyte;
                break;
            case L2_STATE_GET_CRC1:
                l2_state = L2_STATE_SEARCH_FOR_STOP;
                pkt->trailer.crc |= databyte << 8;
                pkt->calculated_crc = crc16(0, &pkt->hdr.start, pkt->payload_len + 3);
                break;
            case L2_STATE_SEARCH_FOR_STOP:
                if (databyte == STOP_BYTE) {
                    l2_state = L2_STATE_SEARCH_FOR_START;
                    pkt->trailer.end = databyte;
                    pkt_received = 1;
                }
                break;
        }
    }
    
    if (context->debuglevel > 0) {
        print_mm_packet(RX, pkt);
    }

    if (pkt->hdr.flags & FLAG_DISCONNECT) {
        printf("Receved disconnect flag!\n");
        context->tx_seq = 0;
        if (context->use_modem == 1) {
            printf("Hanging up modem.\n");                
            hangup_modem(context->fd);
            context->connected = 0;
            return 0;
        }
    }

    if (context->use_modem == 0) {
        if(feof(context->bytestream)) {
            printf("Error: Terminating due to EOF!\n");
            fflush(stdout);
            exit(0);
            return -1;
        }
    }
    return 0;
}   

/* Send manager packet to the terminal.
 *
 * If payload is NULL, an ACK packet will be sent.
 * If payload is not NULL, and the length is 0, a NULL packet will be sent.
 * If payload is not NULL, and length is > 0, then the terminal's phone number
 * will be prepended to the payload and sent.
 */
int send_mm_packet(mm_context_t *context, uint8_t *payload, int len)
{
    mm_packet_t pkt;
    
    if (context->debuglevel > 3) {
        if (payload != NULL) {
            printf("T<--M Sending packet: Terminal: %s, tx_seq=%d\n", context->phone_number, context->tx_seq);
        } else {
            printf("T<--M Sending ACK: rx_seq=%d\n", context->rx_seq);
        }
    }

    pkt.hdr.start = START_BYTE;

    if (payload != NULL) {
        /* Flags for regular TX packet use tx_seq. */
        pkt.hdr.flags = (context->tx_seq & FLAG_SEQUENCE);
        pkt.payload_len = len + PKT_TABLE_ID_OFFSET; /* add room for the phone number. */

        for (int i=0; i < PKT_TABLE_ID_OFFSET; i++) {
            pkt.payload[i]  = (context->phone_number[i * 2    ] - '0') << 4;
            pkt.payload[i] |= (context->phone_number[i * 2 + 1] - '0');
        }
        if(len > 0) {
            memcpy(&pkt.payload[PKT_TABLE_ID_OFFSET], payload, len);
        }
    } else {
        pkt.payload_len = 0;
        /* If payload is NULL, send an ACK packet instead, using rx_seq. */
        pkt.hdr.flags = FLAG_ACK | (context->rx_seq & FLAG_SEQUENCE);
    }

    pkt.hdr.pktlen = pkt.payload_len + 5;
    pkt.trailer.crc = crc16(0, &pkt.hdr.start, pkt.hdr.pktlen - 2);
    pkt.trailer.end = STOP_BYTE;
    pkt.calculated_crc = pkt.trailer.crc;
    /* Copy the CRC and STOP_BYTE to be adjacent to the filled portion of the payload */
    memcpy(&(pkt.payload[pkt.payload_len]), &pkt.trailer.crc, 3);

    if (context->debuglevel > 0) {
        print_mm_packet(TX, &pkt);
    }

    if (context->debuglevel > 2) {
        printf("\n\nRaw Packet: ");
        dump_hex(&pkt.hdr.start, pkt.hdr.pktlen+1);
    }

    if (context->use_modem == 1) {
        write(context->fd, &pkt.hdr.start, pkt.hdr.pktlen+1);
        tcdrain(context->fd);
    }

    for(int i=0; i < pkt.hdr.pktlen+1; i++) {
        if (context->logstream != NULL) fprintf(context->logstream, "UART: TX: %02X\n", ((uint8_t *)(&pkt.hdr.start))[i]);
    }
    if (context->logstream != NULL) fflush(context->logstream);

    if (payload != NULL) {
        context->tx_seq++;
    } else {
        context->rx_seq++;
    }
    return 0;
}

int send_mm_ack(mm_context_t *context)
{
    return (send_mm_packet(context, NULL, 0));
}

int wait_for_mm_ack(mm_context_t *context)
{
    mm_packet_t pkt;

    if(receive_mm_packet(context, &pkt) == 0) {
        if(pkt.payload_len == 0) {
            if(pkt.hdr.flags & FLAG_ACK) {
            } else {
                printf("\tERROR: Got NULL packet without ACK flag set!\n");
                context->tx_seq = 0;
           }
        }
        return 0;
    }
    printf("Error getting ACK, returning -1\n");
    fflush(stdout);
    return -1;
}

int print_mm_packet(int direction, mm_packet_t *pkt)
{

    uint8_t ascii[32];
    uint8_t *pascii = ascii;
    int i;
    int status = 0;

    /* Decode flags: bit 3 = Req/Ack, 2=Retry, 1:0=Sequence. */
    printf("\n%s %s: flags=%02x [%s | %s | Seq:%d], len=%3d (datalen=%3d), crc=%04x.\n",
        (direction == RX) ? "T-->M" : "T<--M",
        (direction == RX) ? "RX" : "TX",
        pkt->hdr.flags,
        (pkt->hdr.flags & FLAG_ACK) ? "ACK" : "REQ",
        (pkt->hdr.flags & FLAG_RETRY) ? "RETRY" : " --- ",
        (pkt->hdr.flags & FLAG_SEQUENCE),
        pkt->hdr.pktlen, pkt->payload_len, pkt->trailer.crc);

    if(pkt->trailer.crc != pkt->calculated_crc) {
        printf("\t*** CRC ERROR, calculated: %04x ***\n", pkt->calculated_crc);
        status = -1;
    }
    if(pkt->trailer.end != STOP_BYTE) {
        printf("\t*** FRAMING ERROR, expected STOP=0x03, got STOP=%02x ***\n", pkt->trailer.end);
        status = -1;
    }

    if (pkt->payload_len > 0) {
        dump_hex(pkt->payload, pkt->payload_len);
    }           

    return status;
}
