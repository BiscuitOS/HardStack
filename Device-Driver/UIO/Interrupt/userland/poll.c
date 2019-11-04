/*
 * UIO Interrupt Userland
 *
 * (C) 2019.10.30 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <poll.h>

#define UIO_DEV		"/dev/uio0"

int main()
{
	struct pollfd fds;
	int fd, ret;

	fd = open(UIO_DEV, O_RDWR);
	if (fd < 0) {
		printf("Error: Unable to open %s\n", UIO_DEV);
		return -1;
	}

	while (1) {
		uint32_t info = 1;

		/* Enable Interrupt */
		info = 1;
		write(fd, &info, sizeof(info));

		fds.fd = fd;
		fds.events = POLLIN;

		ret = poll(&fds, 1, -1);
		/* Disable Interrupt */
		info = 0;
		write(fd, &info, sizeof(info));
		if (ret >= 1) {
			/* Handle Interrupt */
			printf("Interrup from kernel\n");
			sleep(1);
		} else {
			printf("Error: polling\n");
			close(fd);
			return -1;
		}
	}
	
	close(fd);
	
	return 0;
}
