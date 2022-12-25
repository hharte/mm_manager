/*
 * UDP Packet Defintions, part of mm_manager.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2022, Howard M. Harte
 */

#ifndef MM_UDP_H_
#define MM_UDP_H_

#ifndef _WIN32
#define SOCKET          int
#define INVALID_SOCKET  (-1)
#define SOCKET_ERROR    (-1)
#endif  /* _WIN32 */

#define MM_UDP_PORT 27273	/* UDP port to send Millennium packets */

int mm_create_udp(const char* ip_addr, uint16_t port);
int mm_udp_send_pkt(int direction, mm_packet_t* pkt);
int mm_close_udp(void);

#endif /* MM_UDP_H_ */

