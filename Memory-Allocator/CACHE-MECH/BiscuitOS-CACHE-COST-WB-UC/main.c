/*
 * Memory Type COST: WB and UC
 *
 * (C) 2023.02.19 <buddy.zhang@aliyun.com>
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

#define DEV_PATH		"/dev/BiscuitOS-CACHE"
#define PAGE_SIZE		(4 * 1024)

enum page_cache_mode {
	_PAGE_CACHE_MODE_WB       = 0,
	_PAGE_CACHE_MODE_WC       = 1,
	_PAGE_CACHE_MODE_UC_MINUS = 2,
	_PAGE_CACHE_MODE_UC       = 3,
	_PAGE_CACHE_MODE_WT       = 4,
	_PAGE_CACHE_MODE_WP       = 5,
	_PAGE_CACHE_MODE_NUM      = 8
};

static void memory_test_costtime(char *mem, const char *CASE)
{
	unsigned long count = 900000;
	struct timeval tv, ntv;

	gettimeofday(&tv, NULL);
	while (count--) {
		/* READ */
		char a = mem[0];
		/* WRITE */
		mem[0] = 'a';
	}
	gettimeofday(&ntv, NULL);
	printf("%s Cost Time: %ld nsec\n", CASE, ntv.tv_sec * 1000000 +
			ntv.tv_usec - tv.tv_sec * 1000000 - tv.tv_usec);
}

static void *BiscuitOS_alloc(int pcm, unsigned long size, int fd)
{
	/* mmap */
	void *base = mmap(NULL, size, PROT_READ | PROT_WRITE,
				  	MAP_PRIVATE, fd, pcm << 12);
	if (base == MAP_FAILED) {
		printf("ERROR: mmaping failed.\n");
		exit(-1); /* Force exit */
	}
	return base;
}

int main()
{
	void *base0, *base1;
	int fd;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	base0 = BiscuitOS_alloc(_PAGE_CACHE_MODE_UC, PAGE_SIZE, fd);
	base1 = BiscuitOS_alloc(_PAGE_CACHE_MODE_WB, PAGE_SIZE, fd);

	memory_test_costtime(base0, "UC");
	memory_test_costtime(base1, "WB");

	munmap(base0, PAGE_SIZE);
	munmap(base1, PAGE_SIZE);
	close(fd);

	return 0;
}
