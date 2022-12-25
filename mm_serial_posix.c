/*
 * POSIX serial port library, part of mm_manager.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 */

#include <stdio.h>  /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h> /* String function definitions */
#include <fcntl.h>  /* File control definitions */
#include <errno.h>  /* Error number definitions */
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "mm_serial.h"

/*
 * Open serial port specified in modem_dev.
 *
 * Returns the file descriptor on success or -1 on error.
 */
int platform_open_serial(const char *modem_dev) {
    int fd;

    fd = open(modem_dev, O_RDWR | O_NOCTTY | O_NDELAY | O_SYNC);

    if (fd != -1) {
        if (fcntl(fd, F_SETFL, 0) != 0) {
            close(fd);
            return -1;
        }
    }
    return fd;
}

/* Initialize Serial Port options */
int platform_init_serial(int fd, int baudrate) {
    struct termios options;
    speed_t speed;

    switch (baudrate) {
        case 1200:
            speed = B1200;
            break;
        case 2400:
            speed = B2400;
            break;
        case 4800:
            speed = B4800;
            break;
        case 9600:
            speed = B9600;
            break;
        case 19200:
            speed = B19200;
            break;
        case 38400:
            speed = B38400;
            break;
        case 57600:
            speed = B57600;
            break;
        case 115200:
            speed = B115200;
            break;
        case 230400:
            speed = B230400;
            break;
        default:
            printf("%s: Invalid baud rate: %d, defaulting to 1200.\n",
                   __FUNCTION__, baudrate);
            speed = B1200;
            break;
    }

    /* Set serial port options. */
    cfmakeraw(&options);
    options.c_cflag |= (CS8 | CLOCAL | CREAD | CRTSCTS | HUPCL);
    options.c_cflag &= ~(PARODD | CSTOPB);
    options.c_lflag &= ~(ECHOCTL | ECHOPRT | FLUSHO);
    options.c_iflag |= (IGNPAR | IGNBRK);

#ifdef __APPLE__
    options.c_cflag &= ~(CDTR_IFLOW | CDSR_OFLOW);
    options.c_lflag &= ~(NOKERNINFO);
#endif /* __APPLE__ */

    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 10;

    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);

    tcsetattr(fd, TCSANOW, &options);

    return 0;
}

int platform_close_serial(int fd) {
    return close(fd);
}

ssize_t platform_read_serial(int fd, void *buf, size_t count) {
    return read(fd, buf, count);
}

ssize_t platform_write_serial(int fd, const void *buf, size_t count) {
    return write(fd, buf, count);
}

int platform_drain_serial(int fd) {
    return tcdrain(fd);
}

int platform_flush_serial(int fd) {
    return tcflush(fd, TCIOFLUSH);
}

int platform_serial_set_dtr(int fd, int set) {
    int status = 0;

    ioctl(fd, TIOCMGET, &status);

    if (set) {
        status |= TIOCM_DTR;
    } else {
        status &= ~TIOCM_DTR;
    }
    ioctl(fd, TIOCMSET, status);

    return 0;
}

int platform_serial_get_modem_status(int fd) {
    int status = 0;
    int retstatus = 0;

    ioctl(fd, TIOCMGET, &status);

    if (status & TIOCM_CTS) {
        retstatus |= 0x0010;
    }

    if (status & TIOCM_DSR) {
        retstatus |= 0x0020;
    }

    if (status & TIOCM_RNG) {
        retstatus |= 0x0040;
    }

    if (status & TIOCM_CAR) {
        retstatus |= 0x0080;
    }

    return retstatus;
}
