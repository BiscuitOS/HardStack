/*
 * Anonymout private mmap on Userspace
 *
 * (C) 2021.04.02 BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define BISCUITOS_MAP_SIZE	4096
/* PCI 0x1016:0x1413 */
#define MMIO_BASE		0xF0000000

int main()
{
	unsigned long *val;
	char *base;
	int fd;

	/* Open /dev/mem */
	fd = open("/dev/mem", O_RDWR);
	if (fd < 0) {
		printf("ERROR: Open /dev/mem failed.\n");
		return -EBUSY;
	}

	/* MMAP MMIO */
	base = (char *)syscall(9, NULL, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED,
			    fd,
			    MMIO_BASE);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -ENOMEM;
	}

	val = (unsigned long *)base;
	/* Trigger page fault */
	*val = 88520;
	printf("%#lx => %ld\n", (unsigned long)val, *val);

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);
	close(fd);

	return 0;
}
