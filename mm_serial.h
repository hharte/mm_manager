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
#endif /* if defined(_MSC_VER) */

extern int open_serial(const char *modem_dev);
extern int init_serial(int fd, int baudrate);
extern int close_serial(int fd);
ssize_t    read_serial(int fd, void *buf, size_t count);
ssize_t    write_serial(int fd, const void *buf, size_t count);
int        drain_serial(int fd);
int        flush_serial(int fd);

#endif  /* MM_SERIAL_H_ */
