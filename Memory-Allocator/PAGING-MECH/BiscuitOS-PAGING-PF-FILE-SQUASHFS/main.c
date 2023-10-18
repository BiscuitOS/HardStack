// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with File-Mapped SQUASHFS
 *
 * (C) 2023.09.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(4096 * 6)
#define MAP_VADDR	(0x6000000000)
#define FILE_PATH	"/mnt/squashfs/BiscuitOS.txt"

int main()
{
	char *base, ch;
	int fd;

	fd = open(FILE_PATH, O_RDONLY);
	if (fd < 0) {
		printf("%s Open Failed.\n", FILE_PATH);
		exit (-1);
	}

	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ,
		    MAP_SHARED,
		    fd,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* READ Ops, Trigger #PF FAULT-AROUND */
	ch = *base;
	printf("SQUASHFS %#lx => %c\n", (unsigned long)base, ch);

	munmap(base, MAP_SIZE);
	close(fd);

	return 0;
}
