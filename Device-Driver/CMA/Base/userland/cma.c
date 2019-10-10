/*
 * Contiguous Memory Allocate Application
 *
 * (C) 2019.10.08 <buddy.zhang@aliyun.com>
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
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdarg.h>

#define CMA_MEM_ALLOCATE	_IOW('m', 1, unsigned int)
#define CMA_MEM_RELEASE		_IOW('m', 2, unsigned int)
#define CMA_PATH		"/dev/CMA_demo"

struct CMA_demo_region {
	unsigned long virt;
	unsigned long phys;
	unsigned long offset;
	unsigned long length;
};

int main()
{
	struct CMA_demo_region region;
	void *base;
	int fd;

	fd = open(CMA_PATH, O_RDWR, 0);
	if (fd < 0) {
		printf("Can't open %s\n", CMA_PATH);
		return -1;
	}
	memset(&region, 0, sizeof(region));
	region.length = 1 << 20;
	region.phys = 0; /* auto phys address */

	/* Allocate memory from CMA */
	if (ioctl(fd, CMA_MEM_ALLOCATE, &region) < 0) {
		printf("CMA_MEM_ALLOCATE: ioctl failed\n");
		return -1;
	}

	/* Mmap physical address into user space */
	base = mmap(NULL, region.length, PROT_READ | PROT_WRITE, 
						MAP_SHARED, fd, region.phys);
	if (base == MAP_FAILED) {
		printf("CMA_MMAP: mmap failed\n");
		close(fd);
		return -1;
	}

	strcpy(base, "Hello BiscuitOS");
	/* Information */
	printf("CMA region: %s\n", (char *)base);
	printf("Phys: %#lx - %#lx\n", region.phys, 
					region.phys + region.length);
	printf("Virt: %#lx - %#lx\n", base, 
					(unsigned long)base + region.length);

	/* Unmmap */
	munmap(base, region.length);

	/* Free memory from CMA */
	if (ioctl(fd, CMA_MEM_RELEASE, &region) < 0) {
		printf("CMA_MEM_RELEASE: ioctl failed\n");
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}
