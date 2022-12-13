/*
 * POSIX/Win32 serial port library, part of mm_manager.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 */

#ifndef MM_SERIAL_H_
#define MM_SERIAL_H_

#if defined(_MSC_VER)
# include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else  /* if defined(_MSC_VER) */
# include <unistd.h>
#define MS_RING_ON      0x0040
#define MS_RLSD_ON      0x0080
#endif /* if defined(_MSC_VER) */

typedef struct mm_serial_context {
    int fd;
    FILE *logstream;
    FILE *bytestream;
} mm_serial_context_t;

mm_serial_context_t* open_serial(const char *modem_dev, FILE *logstream, FILE *bytestream);
extern int init_serial(mm_serial_context_t *pserial_context, int baudrate);
extern int close_serial(mm_serial_context_t *pserial_context);
ssize_t    read_serial(mm_serial_context_t *pserial_context, void *buf, size_t count, int inject_error);
ssize_t    write_serial(mm_serial_context_t *pserial_context, const void *buf, size_t count);
int        drain_serial(mm_serial_context_t *pserial_context);
int        flush_serial(mm_serial_context_t *pserial_context);
int        serial_set_dtr(mm_serial_context_t* pserial_context, int set);
int        serial_get_modem_status(mm_serial_context_t* pserial_context);

extern int platform_open_serial(const char *modem_dev);
extern int platform_init_serial(int fd, int baudrate);
extern int platform_close_serial(int fd);
ssize_t    platform_read_serial(int fd, void *buf, size_t count);
ssize_t    platform_write_serial(int fd, const void *buf, size_t count);
int        platform_drain_serial(int fd);
int        platform_flush_serial(int fd);
int        platform_serial_set_dtr(int fd, int set);
int        platform_serial_get_modem_status(int fd);

#endif  /* MM_SERIAL_H_ */
