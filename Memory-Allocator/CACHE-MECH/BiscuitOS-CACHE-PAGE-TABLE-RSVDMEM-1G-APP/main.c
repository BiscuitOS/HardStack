// SPDX-License-Identifier: GPL-2.0
/*
 * Mapping Various Memory Type 1Gig Memory into Userspace
 *
 * (C) 2023.02.19 <buddy.zhang@aliyun.com>
 * (C) 2022.10.16 BiscuitOS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#define DEV_PATH		"/dev/BiscuitOS-MEM-1G"
#define PAGE_SIZE		(1024 * 1024 * 1024)

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
	enum page_cache_mode pcm = _PAGE_CACHE_MODE_UC;
	void *base;
	char *data;
	int fd;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* mmap */
	base = mmap(NULL, PAGE_SIZE,
			  PROT_READ | PROT_WRITE,
			  MAP_PRIVATE,
			  fd,
			  pcm << 12);
	if (base == MAP_FAILED) {
		printf("ERROR: mmaping failed.\n");
		exit(-1); /* Force exit */
	}
	data = (char *)base + PAGE_SIZE / 2;

	/* Use, don't trigger page fault */
	sprintf(data, "Hello BiscuitOS!");
	printf("%#lx: %s\n", (unsigned long)data, data);

	/* for debug: cat /sys/kernel/debug/x86/pat_memtype_list */
	munmap(base, PAGE_SIZE);
	close(fd);

	return 0;
}
