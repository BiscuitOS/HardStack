// SPDX-License-Identifier: GPL-2.0
/*
 * Centralized Mapping: 512G on Signle PMD PAGE
 *
 * (C) 2023.11.22 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAP_SIZE	(0x8000000000ULL) /* 512Gig */
#define FILE_PATH	"/dev/BiscuitOS-CETMAP"

int main()
{
	char *addr, ch;
	int fd;

	/* OPEN DEVICE */
	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: open %s failed.\n", FILE_PATH);
		return -1;
	}

	/* PREALLOC 512G MEMORY */
	addr = mmap(NULL, MAP_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd, 
			0);
	if (addr == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -1;
	}

	/* Random READ Range on 512G, Don't Trigger #PF */
	ch = addr[1024 * 1024 * 1024];
	printf("%#lx => %d\n", (unsigned long)addr, ch);

	/* RECLAIM */
	munmap(addr, MAP_SIZE);
	close(fd);

	return 0;
}
