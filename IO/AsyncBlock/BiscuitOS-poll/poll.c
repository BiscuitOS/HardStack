/*
 * BiscuitOS poll
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
#include <poll.h>

/* PATH for EPOLL Interface */
#define DEV_PATH		"/dev/BiscuitOS"
#define OPEN_MAX		1024

int main()
{
	struct pollfd pfd;
	char buffer[128];
	int fd, nready;
	int rvl = 0;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	pfd.fd = fd;
	pfd.events = POLLIN;

	for (;;) {
		nready = poll(&pfd, 1, -1); /* block wait */
		if (pfd.events == POLLIN && pfd.fd == fd) {
			read(fd, buffer, 64);
			printf("Poll load data....\n");
		}
	}

	rvl = 0;
out:
	close(fd);
	return rvl;
}
