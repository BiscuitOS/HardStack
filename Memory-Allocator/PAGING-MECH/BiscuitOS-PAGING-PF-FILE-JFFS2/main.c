// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with File-Mapped JFFS2 FS
 *
 * (C) 2023.09.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)
#define FILE_PATH	"/mnt/jffs2/BiscuitOS.txt"

int main()
{
	char *base;
	int fd;

	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("%s Open Failed.\n", FILE_PATH);
		exit (-1);
	}

	base = mmap((void *)MAP_VADDR, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Write Ops, Trigger #PF */
	*base = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("JFFS2-FS %#lx => %c\n", (unsigned long)base, *base);

	munmap(base, PAGE_SIZE);
	close(fd);

	return 0;
}
