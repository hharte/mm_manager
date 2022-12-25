/*
 * UDP Packed Send, part of mm_manager.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2022, Howard M. Harte
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif  /* _WIN32 */

#include "./mm_manager.h"
#include "./mm_udp.h"

struct sockaddr_in si_src, si_dst;
SOCKET sock;

int mm_create_udp(const char* ip_addr, uint16_t port) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fprintf(stderr, "%s: WSAStartup() Failed. Error Code : %d", __func__, WSAGetLastError());
        return -1;
    }
#endif /* _WIN32 */

    /* Crate a UDP socket to facilitate live Wireshark capture. */
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
#ifdef _WIN32
        fprintf(stderr, "%s: Failed to create socket: %d\n", __func__, WSAGetLastError());
#else
        fprintf(stderr, "%s: Failed to create socket: %s\n", __func__, strerror(errno));
#endif /* _WIN32 */
        mm_close_udp();
        return -2;
    }

    memset((char *) &si_src, 0, sizeof(si_src));
    si_src.sin_family = AF_INET;
    si_src.sin_port = htons(port);
    si_src.sin_addr.s_addr = INADDR_ANY;

    memset((char *) &si_dst, 0, sizeof(si_dst));
    si_dst.sin_family = AF_INET;
    si_dst.sin_port = htons(port);
    si_dst.sin_addr.s_addr = inet_addr(ip_addr);

    /* Bind socket to port 27273 "CRASE" */
    if( bind(sock , (struct sockaddr*)&si_src, sizeof(si_src) ) == SOCKET_ERROR)
    {
#ifdef _WIN32
        fprintf(stderr, "%s: Failed to bind socket: %d\n", __func__, WSAGetLastError());
#else
        fprintf(stderr, "%s: Failed to bind socket: %s\n", __func__, strerror(errno));
#endif /* _WIN32 */
        mm_close_udp();
        return -3;
    }

    return 0;
}

int mm_udp_send_pkt(int direction, mm_packet_t *pkt) {
    pkt->hdr.start |= (direction == TX) ? 0x80 : 0;

    /* Send payload over UDP to 127.0.0.1:27273 to facilitate live Wireshark capture. */
    if (sendto(sock, (const char *)&pkt->hdr.start, (size_t)pkt->hdr.pktlen + 1, 0, (struct sockaddr *)&si_dst, sizeof(si_dst)) == SOCKET_ERROR)
    {
#ifdef _WIN32
        fprintf(stderr, "%s: sendto() failed: %d\n", __func__, WSAGetLastError());
#else
        fprintf(stderr, "%s: sendto() failed: %s\n", __func__, strerror(errno));
#endif /* _WIN32 */
    }
    pkt->hdr.start &= 0x7F;
    return 0;
}

int mm_close_udp(void) {
    /* close UDP socket */
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif /* _WIN32 */

    return 0;
}

