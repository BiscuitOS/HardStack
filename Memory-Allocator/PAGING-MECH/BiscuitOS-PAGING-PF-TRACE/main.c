// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault Trace
 *
 * (C) 2023.09.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(4096)

int main()
{
	char *base;

	base = mmap(NULL, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Write Ops, Trigger #PF */
	*base = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("%#lx => %c\n", (unsigned long)base, *base);

	munmap(base, MAP_SIZE);

	return 0;
}
