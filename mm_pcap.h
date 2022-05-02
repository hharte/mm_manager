/*
 * Packet Capture (PCAP) Defintions, part of mm_manager.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2022, Howard M. Harte
 */

#ifndef MM_PCAP_H_
#define MM_PCAP_H_

typedef struct mm_pcap_hdr_s {
          uint32_t magic_number;   /* 0xa1b2c3d4 magic number */
          uint16_t version_major;  /* 2: major version number */
          uint16_t version_minor;  /* 4: minor version number */
          int32_t  thiszone;       /* 0: GMT to local correction */
          uint32_t sigfigs;        /* 0: accuracy of timestamps */
          uint32_t snaplen;        /* 1024 max length of captured packets, in octets */
          uint32_t network;        /* LINKTYPE_USER0: data link type */
 } mm_pcap_hdr_t;

typedef struct mm_pcaprec_hdr_s {
          uint32_t ts_sec;         /* timestamp seconds */
          uint32_t ts_usec;        /* timestamp microseconds */
          uint32_t incl_len;       /* number of octets of packet saved in file */
          uint32_t orig_len;       /* actual length of packet */
 } mm_pcaprec_hdr_t;

#endif /* MM_PCAP_H_ */
