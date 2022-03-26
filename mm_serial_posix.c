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

/*
 * Open serial port specified in modem_dev.
 *
 * Returns the file descriptor on success or -1 on error.
 */
int open_serial(const char *modem_dev) {
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
int init_serial(int fd, int baudrate) {
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

int close_serial(int fd) {
    return close(fd);
}

ssize_t read_serial(int fd, void *buf, size_t count) {
    return read(fd, buf, count);
}

ssize_t write_serial(int fd, const void *buf, size_t count) {
    return write(fd, buf, count);
}

int drain_serial(int fd) {
    return tcdrain(fd);
}

int flush_serial(int fd) {
    return tcflush(fd, TCIOFLUSH);
}
