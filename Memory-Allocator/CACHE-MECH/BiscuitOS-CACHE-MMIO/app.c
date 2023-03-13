/*
 * Mmapping Variable Memory Type for MMIO on Userspace
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
#define DEV_PATH		"/dev/BiscuitOS-MMIO"
#define MMIO_BASE		0xF0000000
#define MMIO_SIZE		4096

enum page_cache_mode {
	_PAGE_CACHE_MODE_WB       = 0,
	_PAGE_CACHE_MODE_WC       = 1,
	_PAGE_CACHE_MODE_UC_MINUS = 2,
	_PAGE_CACHE_MODE_UC       = 3,
	_PAGE_CACHE_MODE_WT       = 4,
	_PAGE_CACHE_MODE_WP       = 5,
	_PAGE_CACHE_MODE_NUM      = 8
};

int main()
{
	enum page_cache_mode pcm = _PAGE_CACHE_MODE_WT;
	void *base;
	int fd;

	/* Open /dev/mem */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Open %s failed.\n", DEV_PATH);
		return -EBUSY;
	}

	/* MMAP MMIO: Alloc virtual memory and build Paging-Table */
	base = (void *)mmap(NULL, 
			    MMIO_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED,
			    fd,
			    pcm << 12);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -ENOMEM;
	}

	/* Don't trigger page-fault */
	sprintf((char *)base, "Hello BiscuitOS");
	printf("%#lx => %s\n", (unsigned long)base, (char *)base);

	/* Just for debug */
	sleep(-1);

	/* unmap */
	munmap(base, MMIO_SIZE);
	close(fd);

	return 0;
}
