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

    fd = open(modem_dev, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        /* Could not open the port. */
        printf("open_port: Unable to open %s.", modem_dev);
    } else {
        fcntl(fd, F_SETFL, 0);
    }
    return (fd);
}

/* Initialize Serial Port options */
int init_port(int fd, int baudrate)
{
    struct termios options;

    /* get the current options */
    tcgetattr(fd, &options);

    /* set raw input, 1 second timeout */
    options.c_cflag     |= (CLOCAL | CREAD);
    options.c_cflag     |= CRTSCTS;   /* Enable RTS/CTS Flow Control */
    options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag     &= ~OPOST;
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 10;

    /* set the options */
    tcsetattr(fd, TCSANOW, &options);

    /*
    * Get the current options for the port...
    */
    tcgetattr(fd, &options);

    /*
    * Set the baud rates to specified baudrate...
    */
    cfsetispeed(&options, baudrate);
    cfsetospeed(&options, baudrate);

    /*
    * Enable the receiver and set local mode...
    */
    options.c_cflag |= (CLOCAL | CREAD);

    /*
    * Set the new options for the port...
    */
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

    printf("Set modulation to Bell 212.\n");
    status = send_at_command(fd, "AT+MS=B212");
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
    char *bufptr;       /* Current char in buffer */
    int  nbytes;        /* Number of bytes read */
    int  tries = 0;     /* Number of tries so far */

    tcflush(fd, TCIOFLUSH);

    while(1) {
        /* read characters into our string buffer until we get a CR or NL */
        bufptr = buffer;
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
    send_at_command(fd, "+++");
    send_at_command(fd, "ATH0");

    return (0);
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
        if (write(fd, buffer, strlen(buffer)) < strlen(buffer)) {
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
