// SPDX-License-Identifier: GPL-2.0
/*
 * SMAPS with FilePmdMapped Memory
 *
 * (C) 2023.08.25 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define DEV_PATH		"/dev/BiscuitOS-PageTable"
#define PAGE_SIZE		(2 * 1024 * 1024)

int main()
{
	void *base;
	int fd;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	base = mmap(NULL, PAGE_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			0);
	if (base == MAP_FAILED) {
		printf("ERROR: No free Memory\n");
		close(fd);
		return -1;
	}

	/* Write Ops, Dont trigger PageFault */
	*(char *)base = 'B';
	/* Read Ops. Dont trigger PageFault */
	printf("%#lx => %c\n", (unsigned long)base, *(char *)base);

	sleep(-1); /* Just for debug */
	munmap(base, PAGE_SIZE);
	close(fd);

	return 0;
}
