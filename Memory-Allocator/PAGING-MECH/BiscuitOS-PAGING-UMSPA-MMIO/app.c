// SPDX-License-Identifier: GPL-2.0
/*
 * UMSPA: MAPPING MMIO
 *
 * (C) 2023.11.03 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAP_SIZE	(4 * 1024)
#define FILE_PATH	"/dev/mem"
#define MMIO_BASE	0xF0000000UL

int main()
{
	void *addr;
	int fd;

	/* OPEN FILE */
	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: open %s failed.\n", FILE_PATH);
		return -1;
	}

	/* MAPPING */
	addr = mmap(NULL, MAP_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd, 
			MMIO_BASE);
	if (!addr) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -1;
	}

	/* ACCESS */
	*(char *)addr = 'B';
	printf("UMSPA %#lx => %c\n", (unsigned long)addr, *(char *)addr);

	/* RECLAIM */
	munmap(addr, MAP_SIZE);
	close(fd);

	return 0;
}
