// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with File-Mapped VFAT COW
 *
 * (C) 2023.10.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)
#define FILE_PATH	"/mnt/vfat/BiscuitOS.txt"

int main()
{
	char *base, ch;
	int fd;

	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("%s Open Failed.\n", FILE_PATH);
		exit (-1);
	}

	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE,
		    fd,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Read Ops, Trigger #PF bind PAGECACHE */
	ch = *base;

	/* Write Ops, Trigger #PF COW to COPY PAGE */
	*base = 'E';

	printf("VFAT-COW: %#lx => %c\n", (unsigned long)base, ch);

	munmap(base, MAP_SIZE);
	close(fd);

	return 0;
}
