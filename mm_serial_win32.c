/*
 * WIN32 serial port library, part of mm_manager.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 */

#include <windows.h>
#include <stdio.h>
#include <stdint.h>

#include "./mm_manager.h"

typedef SSIZE_T ssize_t;

static HANDLE hHandleTable[10] = { 0 };

int platform_open_serial(const char *modem_dev) {
    HANDLE hComm;
    int    fd = 0; /* FIXME: only one serial connection allowed. */

    hComm = CreateFileA(modem_dev, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    hHandleTable[0] = hComm;

    if (hComm == INVALID_HANDLE_VALUE) {
        return -1;
    }

    return fd;
}

int platform_close_serial(int fd) {
    HANDLE hComm = hHandleTable[fd];

    return CloseHandle(hComm);
}

/* Initialize Serial Port options */
int platform_init_serial(int fd, int baudrate) {
    HANDLE hComm = hHandleTable[fd];
    DCB    myDCB;

    switch (baudrate) {
        case 1200:
        case 2400:
        case 4800:
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 115200:
        case 230400:
            break;
        default:
            printf("%s: Invalid baud rate: %d, defaulting to 1200.\n",
                   __FUNCTION__, baudrate);
            break;
    }

    if (GetCommState(hComm, &myDCB)) {
        myDCB.BaudRate = baudrate;

        if (SetCommState(hComm, &myDCB)) {
            COMMTIMEOUTS cto;

            if (GetCommTimeouts(hComm, &cto)) {
                // Set the new timeouts
                cto.ReadIntervalTimeout        = 1000;
                cto.ReadTotalTimeoutConstant   = 1000;
                cto.ReadTotalTimeoutMultiplier = 10;

                if (SetCommTimeouts(hComm, &cto)) {
                    return 0;
                }
            }
        }
    }
    return -1;
}

ssize_t platform_read_serial(int fd, void *buf, size_t count) {
    HANDLE   hComm      = hHandleTable[fd];
    uint32_t bytes_read = 0;
    BOOL     Status;

    Status = ReadFile(hComm, buf, (DWORD)count, (LPDWORD)&bytes_read, NULL);

    if (Status == 0) return 0;
    return (ssize_t)bytes_read;
}

ssize_t platform_write_serial(int fd, const void *buf, size_t count) {
    HANDLE   hComm         = hHandleTable[fd];
    uint32_t bytes_written = 0;
    BOOL     Status;

    Status = WriteFile(hComm, buf, (DWORD)count, (LPDWORD)&bytes_written, NULL);

    if (Status == 0) return 0;

    return (ssize_t)bytes_written;
}

int platform_drain_serial(int fd) {
    HANDLE hComm = hHandleTable[fd];

    if (!FlushFileBuffers(hComm)) {
        return -1;
    }

    return 0;
}

int platform_flush_serial(int fd) {
    HANDLE hComm = hHandleTable[fd];

    if (!PurgeComm(hComm, PURGE_RXCLEAR | PURGE_TXCLEAR)) {
        return -1;
    }

    return 0;
}
