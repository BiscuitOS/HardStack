/*
 * BiscuitOS epoll
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
#include <sys/epoll.h>

/* PATH for EPOLL Interface */
#define DEV_PATH		"/dev/BiscuitOS"
#define OPEN_MAX		1024
#define epoll_err(msg, rvt)	\
({				\
	printf(msg);		\
	rvt = -1;		\
	goto out;		\
})

int main()
{
	struct epoll_event ept, ep[OPEN_MAX];
	int fd, efd, nready, i;
	char buffer[128];
	int rvl = 0;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	efd = epoll_create(OPEN_MAX);
	if (efd < 0)
		epoll_err("epoll_create error.\n", rvl);

	ept.events = EPOLLIN;	/* Only Care EPOLLIN event */
	ept.data.fd = fd;
	rvl = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ept);
	if (rvl < 0)
		epoll_err("epoll_ctl error.\n", rvl);

	for (;;) {
		nready = epoll_wait(efd, ep, OPEN_MAX, -1); /* block wait */
		if (nready < 0)
			epoll_err("epoll_wait error.\n", rvl);

		for (i = 0; i < nready; i++) {
			if (!ep[i].events & EPOLLIN)
				continue;
			if (ep[i].data.fd != fd)
				continue;
			/* Read Data */
			read(fd, buffer, 120);
			printf("Epoll load data....\n");
		}
	}

	rvl = 0;
out:
	close(fd);
	return rvl;
}
