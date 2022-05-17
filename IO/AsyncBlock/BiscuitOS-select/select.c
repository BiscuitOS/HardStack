/*
 * BiscuitOS select
 *
 * (C) 2022.05.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.04.01 BisuitOS 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>

/* PATH for EPOLL Interface */
#define DEV_PATH		"/dev/BiscuitOS"
#define OPEN_MAX		1024

int main()
{
	struct timeval tv;
	char buffer[128];
	int fd, rvl = 0;
	fd_set fds;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	for (;;) {
		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		/* timeout */
		tv.tv_sec  = 1; /* sec */
		tv.tv_usec = 0;

		rvl = select(OPEN_MAX, &fds, NULL, NULL, &tv);
		if (rvl < 0) {
			printf("ERROR select\n");
			rvl = -1;
			goto out;
		} else if (rvl == 0)
			/* Event not ready */
			continue;
		else
			if (FD_ISSET(fd, &fds)) {
				/* Read data */
				read(fd, buffer, 64);
				printf("Select load data....\n");
			}
	}

	rvl = 0;
out:
	close(fd);
	return rvl;
}
