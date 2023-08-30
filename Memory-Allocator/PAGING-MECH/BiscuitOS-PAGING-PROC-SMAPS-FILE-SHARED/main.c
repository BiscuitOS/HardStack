// SPDX-License-Identifier: GPL-2.0
/*
 * SMAPS: File Shared Memory
 *
 * (C) 2023.08.24 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE	(4096 * 4)
#define MAP_VADDR	(0x6000000000)

int main()
{
	char *base;
	int fd;

	fd = open("/tmp/BiscuitOS.txt", O_RDWR);
	if (fd < 0) {
		printf("/tmp/BiscuitOS.txt can't open.\n");
		exit (-1);
	}

	base = mmap((void *)MAP_VADDR, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -1;
	}

	/* Read Ops. Don't Trigger PageFault */
	printf("%#lx => %s\n", (unsigned long)base, (char *)base);

	sleep(-1); /* Just for Debug */
	munmap(base, PAGE_SIZE);
	close(fd);

	return 0;
}
