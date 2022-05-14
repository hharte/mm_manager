/*
 * Packet Capture (PCAP) Defintions, part of mm_manager.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2022, Howard M. Harte
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "./mm_manager.h"
#include "./mm_pcap.h"

int mm_create_pcap(const char* capfilename, FILE **pcapstream) {
    mm_pcap_hdr_t pcap_hdr = { 0 };

    if (!(*pcapstream = fopen(capfilename, "wb"))) {
        *pcapstream = NULL;
        return -1;
    }

    pcap_hdr.magic_number = 0xa1b2c3d4;
    pcap_hdr.version_major = 2;
    pcap_hdr.version_minor = 4;
    pcap_hdr.thiszone      = 0;
    pcap_hdr.sigfigs       = 0;
    pcap_hdr.snaplen       = 1024;
    pcap_hdr.network       = 147; // LINKTYPE_USER0

    fwrite(&pcap_hdr, sizeof(mm_pcap_hdr_t), 1, *pcapstream);

    return 0;
}

int mm_add_pcap_rec(FILE* pcapstream, int direction, mm_packet_t *pkt, uint32_t ts_sec, uint32_t ts_usec) {
    mm_pcaprec_hdr_t pcap_rec = { 0 };
    struct timespec ts;

    if (pcapstream == NULL) {
        return -1;
    }

    if (ts_sec == 0 && ts_usec == 0) {
        if (timespec_get(&ts, TIME_UTC) != TIME_UTC)
        {
            fputs("timespec_get failed!", stderr);
            return -1;
        } else {
            ts_sec = (uint32_t)ts.tv_sec;
            ts_usec = ts.tv_nsec / 1000;
        }
    }

    pcap_rec.ts_sec   = ts_sec;
    pcap_rec.ts_usec  = ts_usec;
    pcap_rec.incl_len = pkt->hdr.pktlen + 1;
    pcap_rec.orig_len = pkt->hdr.pktlen + 1;

    /* Write PCAP record header */
    fwrite(&pcap_rec, sizeof(mm_pcaprec_hdr_t), 1, pcapstream);
    pkt->hdr.start |= (direction == TX) ? 0x80 : 0;
    /* Write Payload */
    fwrite(&pkt->hdr.start, (size_t)pkt->hdr.pktlen + 1, 1, pcapstream);
    pkt->hdr.start &= (direction == TX) ? 0x7F : 0;
    return 0;
}

int mm_close_pcap(FILE* pcapstream) {
    if (pcapstream != NULL) {
        fclose(pcapstream);
    }
    return 0;
}