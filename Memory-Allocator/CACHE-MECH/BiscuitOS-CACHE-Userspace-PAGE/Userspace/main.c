/*
 * CACHE Mode for Userspace Page on BiscuitOS
 *
 * (C) 2023.02.08 <buddy.zhang@aliyun.com>
 * (C) 2022.10.16 BiscuitOS
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
#include <sys/types.h>
#include <sys/mman.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>

/* IOCTL */
#define BISCUITOS_CACHE_IO	0xBD
#define BSIO_CACHE_MODE_WB	_IO(BISCUITOS_CACHE_IO, 0x00)
#define BSIO_CACHE_MODE_WC	_IO(BISCUITOS_CACHE_IO, 0x01)
#define BSIO_CACHE_MODE_UC_MIN	_IO(BISCUITOS_CACHE_IO, 0x02)
#define BSIO_CACHE_MODE_UC	_IO(BISCUITOS_CACHE_IO, 0x03)
#define BSIO_CACHE_MODE_WT	_IO(BISCUITOS_CACHE_IO, 0x04)
#define BSIO_CACHE_MODE_WP	_IO(BISCUITOS_CACHE_IO, 0x05)

#define DEV_PATH		"/dev/BiscuitOS-CACHE"
#define MMAP_SIZE		(4 * 1024)
int main()
{
	void *base;
	int fd, r;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* Setup Page CACHE mode */
	r = ioctl(fd, BSIO_CACHE_MODE_WB, (unsigned long)0);
	if (r < 0) {
		printf("ERROR: Ioctl failed.\n");
		close(fd);
		return r;
	}

	/* mmap */
	base = mmap(NULL,
		    MMAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE,
		    fd,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmaping failed.\n");
		close(fd);
		return -1;
	}

	/* Trigger page-fault */
	*(char *)base = 'c';

	/* Test-Self-Memory */
	munmap(base, MMAP_SIZE);
	close(fd);

	return 0;
}
