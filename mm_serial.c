/*
 * Serial port library, part of mm_manager.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 */

#include <errno.h>
#include <stdio.h>  /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h> /* String function definitions */

#include "./mm_serial.h"

/*
 * Open serial port specified in modem_dev.
 *
 * Returns the file descriptor on success or -1 on error.
 */
mm_serial_context_t* open_serial(const char *modem_dev, FILE *logstream, FILE *bytestream) {
    int fd = -1;
    mm_serial_context_t *pserial_context;

    if (bytestream == NULL) {
        fd = platform_open_serial(modem_dev);
    }

    pserial_context = (mm_serial_context_t *)calloc(1, sizeof(mm_serial_context_t));

    if (pserial_context == NULL) {
        fprintf(stderr, "%s: Error allocating memory.\n", __FUNCTION__);
        exit(-ENOMEM);
    }

    pserial_context->fd = fd;
    pserial_context->logstream  = logstream;
    pserial_context->bytestream = bytestream;

    return pserial_context;
}

int close_serial(mm_serial_context_t *pserial_context) {
    int status = -1;

    if (pserial_context != NULL) {
        status = platform_close_serial(pserial_context->fd);
        free(pserial_context);
    }

    return status;
}

int init_serial(mm_serial_context_t *pserial_context, int baudrate) {
    int status = 0;

    if (pserial_context->bytestream == NULL) {
        status = platform_init_serial(pserial_context->fd, baudrate);
    }

    return status;
}

ssize_t read_serial(mm_serial_context_t *pserial_context, void *buf, size_t count, int inject_error) {
    ssize_t bytes_read = -1;
    char testbuf[80];
    char* bytep;
    char databyte;

    if (pserial_context->bytestream == NULL) {
        bytes_read = platform_read_serial(pserial_context->fd, buf, count);
        if (inject_error) {
            printf("Invert RX data\n");
            /* Force an error by inverting the recevied data */
            ((uint8_t *)buf)[0] = ~((uint8_t*)buf)[0];
        }
    }
    else {
        for (int i = 0; i < count; i++) {
            if (feof(pserial_context->bytestream)) {
                printf("%s: Terminating due to EOF.\n", __FUNCTION__);
                fflush(stdout);
                fclose(pserial_context->bytestream);
                exit(0);
            }
            else {
                fgets(testbuf, 80, pserial_context->bytestream);

                /* Data that came from the Millennium Terminal. */
                if ((bytep = strstr(testbuf, "RX: ")) != NULL) {
                    uint32_t filebyte;
                    if (sscanf(bytep, "RX: %x", &filebyte) != 1) {
                        fprintf(stderr, "%s: Error parsing bytestream\n", __FUNCTION__);
                    }
                    databyte = filebyte & 0xFF;
                    ((uint8_t*)buf)[i] = databyte;
                }
            }
        }
        bytes_read = count;
    }

    if (pserial_context->logstream != NULL) {
        for (int i = 0; i < count; i++) {
            fprintf(pserial_context->logstream, "UART: RX: %02X\n", ((uint8_t*)buf)[i]);
        }
    }
    return bytes_read;
}

ssize_t write_serial(mm_serial_context_t *pserial_context, const void *buf, size_t count) {
    ssize_t bytes_written = count;

    if (pserial_context->logstream != NULL) {
        for (int i = 0; i < count; i++) {
            fprintf(pserial_context->logstream, "UART: TX: %02X\n", ((uint8_t*)buf)[i]);
        }
    }

    /* If we are using a serial port, send the data */
    if (pserial_context->bytestream == NULL) {
        bytes_written = platform_write_serial(pserial_context->fd, buf, count);
    }

    return bytes_written;
}

int drain_serial(mm_serial_context_t *pserial_context) {
    int status = -1;
    if (pserial_context->bytestream == NULL) {
        status = platform_drain_serial(pserial_context->fd);
    }
    return status;
}

int flush_serial(mm_serial_context_t *pserial_context) {
    int status = -1;
    if (pserial_context->bytestream == NULL) {
        status = platform_flush_serial(pserial_context->fd);
    }
    return status;
}

int serial_set_dtr(mm_serial_context_t *pserial_context, int set) {
    int status = -1;
    if (pserial_context->bytestream == NULL) {
        status = platform_serial_set_dtr(pserial_context->fd, set);
    }
    return status;
}

int serial_get_modem_status(mm_serial_context_t* pserial_context) {
    int status = -1;
    if (pserial_context->bytestream == NULL) {
        status = platform_serial_get_modem_status(pserial_context->fd);
    }
    return status;
}
