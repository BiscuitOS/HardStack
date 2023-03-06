/*
 * Mmapping WB MMIO on Userspace
 *
 * (C) 2023.02.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

/* Broiler MMIO Base Device */
#define MMIO_BASE		0xF0000000
#define MMIO_SIZE		4096

int main()
{
	unsigned long *val;
	char *base;
	int fd;

	/* Open /dev/mem */
	fd = open("/dev/BiscuitOS-MMIO", O_RDWR);
	if (fd < 0) {
		printf("ERROR: Open /dev/mem failed.\n");
		return -EBUSY;
	}

	/* MMAP MMIO: Alloc virtual memory and build Paging-Table */
	base = (char *)mmap(NULL, 
			    MMIO_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED,
			    fd,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -ENOMEM;
	}

	/* Don't trigger page-fault */
	val = (unsigned long *)base;
	*val = 88520;
	printf("%#lx => %ld\n", (unsigned long)val, *val);

	sleep(-1);

	/* unmap */
	munmap(base, MMIO_SIZE);
	close(fd);

	return 0;
}
