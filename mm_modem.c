/*
 * This is a "Manager" for the Nortel Millennium payhone.
 *
 * It can provision a Nortel Millennium payphone with Rev 1.0 or 1.3
 * Control PCP.  CDRs, Alarms, and Maintenance Reports can also be
 * retieved.
 *
 * www.github.com/hharte/mm_manager
 *
 * (c) 2020-2022, Howard M. Harte
 */

#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>  /* String function definitions */
#include <time.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

/* Static function declarations */
static int send_at_command(int fd, char *command);

/*
 * Open serial port specified in modem_dev.
 *
 * Returns the file descriptor on success or -1 on error.
 */
int open_port(char *modem_dev)
{
    int fd;

    fd = open(modem_dev, O_RDWR | O_NOCTTY | O_NDELAY | O_SYNC);
    if (fd != -1) {
        if (fcntl(fd, F_SETFL, 0) != 0) {
            close(fd);
            return(-1);
        }
    }
    return (fd);
}

/* Initialize Serial Port options */
int init_port(int fd, int baudrate)
{
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
    options.c_cflag |=  (CS8 | CLOCAL | CREAD | CRTSCTS | HUPCL);
    options.c_cflag &= ~(PARODD | CSTOPB);
    options.c_lflag &= ~(ECHOCTL | ECHOPRT | FLUSHO);
    options.c_iflag |=  (IGNPAR | IGNBRK);

#ifdef __APPLE__
    options.c_cflag &= ~(CDTR_IFLOW | CDSR_OFLOW);
    options.c_lflag &= ~(NOKERNINFO);
#endif /* __APPLE__ */

    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 10;

    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);

    tcsetattr(fd, TCSANOW, &options);

    return (0);
}

/* Initialize modem with a series of AT commands */
int init_modem(int fd)
{
    int status;

    printf("Reset modem.\n");
    status = send_at_command(fd, "ATZ");
    if (status != 0) {
        return -1;
    }

    printf("Disable modem command echo.\n");
    status = send_at_command(fd, "ATE=1");
    if (status != 0) {
        return -1;
    }

    printf("Set modulation to Bell 212A.\n");
//    status = send_at_command(fd, "AT&N2");	// 3-Com Business Modem 56K USB (use 1200 baud)
//    status = send_at_command(fd, "ATB1");	// USR 5686 Modem
    status = send_at_command(fd, "AT+MS=B212");	// Lenovo 56K USB Modem
    if (status != 0) {
        return -1;
    }

    printf("Set carrier wait timeout to 3 seconds.\n");
    status = send_at_command(fd, "ATS7=3"); 	/* Wait 3 seconds for carrier. */
    if (status != 0) {
        return -1;
    }

    printf("Set modem autoanswer.\n");
    status = send_at_command(fd, "ATS0=1");

    return(status);
}

/* Wait for modem to connect */
int wait_for_connect(int fd)
{
    char buffer[255];   /* Input buffer */
    int  tries = 0;     /* Number of tries so far */

    tcflush(fd, TCIOFLUSH);

    while(1) {
        int  nbytes;        /* Number of bytes read */

        /* read characters into our string buffer until we get a CR or NL */
        char *bufptr = buffer;    /* Current char in buffer */
        while ((nbytes = read(fd, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0)
        {
        bufptr += nbytes;
        if (bufptr[-1] == '\n' || bufptr[-1] == '\r')
            break;
        }

        /* null terminate the string and see if we got a CONNECT response */
        *bufptr = '\0';

        if (strstr(buffer, "CONNECT") != 0) {
            return (0);
        }
        tries ++;

        if (tries > 1000) break;
    }
    return (-1);
}

int hangup_modem(int fd)
{
    char buffer[80];   /* Input buffer */
    int  tries;        /* Number of tries so far */

    for (tries = 0; tries < 3; tries ++) {
        char *bufptr;      /* Current char in buffer */
        int  nbytes;       /* Number of bytes read */

        tcflush(fd, TCIOFLUSH);

        for (int i = 0; i < 3; i++) {
            write(fd, "+", 1);
            nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
        }

        sleep(1);   /* Some modems need time to process the AT command. */

        /* read characters into our string buffer until we get a CR or NL */
        bufptr = buffer;

        while ((nbytes = read(fd, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0) {
            bufptr += nbytes;
            if (bufptr[-1] == '\n' || bufptr[-1] == '\r') {
                break;
            }
        }

        /* null terminate the string and see if we got an OK response */
        *bufptr = '\0';

        if (strstr(buffer, "OK") != 0) {
            return(send_at_command(fd, "ATH0"));
        }
    }
    return (-1);
}

/* Send AT Command to Modem */
static int send_at_command(int fd, char *command)
{
    char buffer[80];  /* Input buffer */
    char *bufptr;      /* Current char in buffer */
    int  nbytes;       /* Number of bytes read */
    int  tries;        /* Number of tries so far */

    for (tries = 0; tries < 3; tries ++) {
        tcflush(fd, TCIOFLUSH);
        snprintf(buffer, sizeof(buffer), "%s\r", command);

        /* send an AT command followed by a CR */
        if (write(fd, buffer, strnlen(buffer, sizeof(buffer))) < strnlen(buffer, sizeof(buffer))) {
            continue;
        }

        sleep(1);   /* Some modems need time to process the AT command. */

        /* read characters into our string buffer until we get a CR or NL */
        bufptr = buffer;

        while ((nbytes = read(fd, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0) {
            bufptr += nbytes;
            if (bufptr[-1] == '\n' || bufptr[-1] == '\r') {
                break;
            }
        }

        /* null terminate the string and see if we got an OK response */
        *bufptr = '\0';

        if (strstr(buffer, "OK") != 0)
        return (0);
    }
    return (-1);
}
