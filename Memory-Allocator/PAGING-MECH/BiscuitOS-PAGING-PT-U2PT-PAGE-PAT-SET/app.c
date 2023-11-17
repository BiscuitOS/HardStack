// SPDX-License-Identifier: GPL-2.0
/*
 * User with PageTable: SET _PAGE_PAT/_PAGE_PCD/_PAGE_PWT
 *
 * (C) 2023.11.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#define DEV_PATH		"/dev/BiscuitOS-PAT"
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

int main()
{
	enum page_cache_mode pcm = _PAGE_CACHE_MODE_WC;
	void *base;
	int fd;

	/* OPEN DEVICE FILE */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* MMAPING FILE */
	base = mmap(NULL, PAGE_SIZE,
			  PROT_READ | PROT_WRITE,
			  MAP_PRIVATE,
			  fd,
			  pcm << 12);
	if (base == MAP_FAILED) {
		printf("ERROR: mmaping failed.\n");
		exit(-1); /* Force exit */
	}

	/* Use, don't trigger page fault */
	sprintf((char *)base, "Hello BiscuitOS!");
	printf("PAT: %#lx => %s\n", (unsigned long)base, (char *)base);

	munmap(base, PAGE_SIZE);
	close(fd);

	return 0;
}
