// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with File-Mapped F2FS FAULT-AROUND
 *
 * (C) 2023.10.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(4096 * 6)
#define MAP_VADDR	(0x6000000000)
#define FILE_PATH	"/mnt/f2fs/BiscuitOS.txt"

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
		    MAP_SHARED,
		    fd,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Read Ops, Trigger #PF with FAULT AROUND */
	ch = *base;

	/* Read Ops, Don't Trigger #PF */
	printf("F2FS %#lx => %c\n", (unsigned long)base, ch);
	
	/* Read Ops, Don't Trigger #PF */
	ch = base[4096];
	printf("F2FS %#lx => %c\n", (unsigned long)base + 4096, ch);

	munmap(base, MAP_SIZE);
	close(fd);

	return 0;
}
