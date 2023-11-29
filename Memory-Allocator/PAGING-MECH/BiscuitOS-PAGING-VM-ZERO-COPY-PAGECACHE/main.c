// SPDX-License-Identifier: GPL-2.0
/*
 * ZERO COPY: PAGE CACHE
 *
 * (C) 2023.11.21 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)
#define FILE_PATH	"/mnt/BiscuitOS.txt"

int main()
{
	char *base;
	int fd;

	/* OPEN FILE */
	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("%s Open Failed.\n", FILE_PATH);
		exit (-1);
	}

	/* MAPPING FILE */
	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* WRITE Ops, Trigger #PF and ACCESS PAGE CACHE */
	*base = 'H';
	/* READ Ops, Don't Trigger #PF */
	printf("ZC-PAGECACHE: %#lx => %s", (unsigned long)base, base);

	/* RECLAIM */
	munmap(base, MAP_SIZE);
	close(fd);

	return 0;
}
