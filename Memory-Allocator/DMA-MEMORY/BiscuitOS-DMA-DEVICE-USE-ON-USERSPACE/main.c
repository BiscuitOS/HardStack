/*
 * DMA Memory on Userspace
 *
 * (C) 2023.02.24 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>

#define BISCUITOS_MAP_SIZE	(4096)
#define BISCUITOS_PATH		"/dev/Broiler-DMA"
#define OPEN_MAX		1024
/* IOCTL CMD */
#define BISCUITOS_IO		0xBD
#define BISCUITOS_PCI_TO_MEM	_IO(BISCUITOS_IO, 0x00)
#define BISCUITOS_MEM_TO_PCI	_IO(BISCUITOS_IO, 0x01)

int main()
{
	struct epoll_event ept, ep[OPEN_MAX];
	int fd, efd, r, count = 0;
	void *addr;

	/* open */
	fd = open(BISCUITOS_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: open %s failed.\n", BISCUITOS_PATH);
		return -1;
	}

	/* EPOLL to Catch DMA MSIX */
	efd = epoll_create(OPEN_MAX);
	if (efd < 0) {
		printf("ERROR: EPOLL Create Failed.\n");
		goto err_epoll;
	}
	ept.events = EPOLLIN;
	ept.data.fd = fd;
	r = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ept);
	if (r < 0) {
		printf("ERROR: EPOLL ADD Failed.\n");
		goto err_epoll;
	}

	/* mmap to DMA */
	addr = mmap(NULL, BISCUITOS_MAP_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd, 
			0);
	if (addr == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		goto err_map;
	}
	memset(addr, 0x00, BISCUITOS_MAP_SIZE);

	/* Read from DMA */
	ioctl(fd, BISCUITOS_PCI_TO_MEM, (unsigned long)0);

	/* Wait MSIX Interrupt from DMA */
	for (;;) {
		int i, nready;

		nready = epoll_wait(efd, ep, OPEN_MAX, -1);
		if (nready < 0) {
			printf("ERROR: EPOLL Wait\n");
			goto err_dma0;
		}

		for (i = 0; i < nready; i++) {
			if (!ep[i].events & EPOLLIN)
				continue;
			if (ep[i].data.fd != fd)
				continue;
			/* Read Data from DMA */
			printf("%s\n", (char *)addr);
			/* Write Data into DMA */
			sprintf((char *)addr, "Hello BiscuitOS %d", count++);
			/* DMA: DDR to PCI */
			ioctl(fd, BISCUITOS_MEM_TO_PCI, (unsigned long)0);
		}
		/* Continue Read From DMA */
		ioctl(fd, BISCUITOS_PCI_TO_MEM, (unsigned long)0);
	}
	
	/* unmap */
	munmap(addr, BISCUITOS_MAP_SIZE);
	close(fd);

	return 0;

err_dma0:
	munmap(addr, BISCUITOS_MAP_SIZE);
err_map:
err_epoll:
	close(fd);
	return -1;
}
