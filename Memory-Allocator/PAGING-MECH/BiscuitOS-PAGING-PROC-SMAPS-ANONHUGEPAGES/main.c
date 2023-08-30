// SPDX-License-Identifier: GPL-2.0
/*
 * SMAPS: Anonymous Huge Memory
 *
 * (C) 2023.08.24 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define HPAGE_SIZE	(4 * 2 * 1024 * 1024)
#define MAP_VADDR	(0x6000000000)

int main()
{
	void *base;

	base = mmap((void *)MAP_VADDR, HPAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS |
		    MAP_FIXED_NOREPLACE,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Write Ops. Trigger PageFault */
	*(char *)base = 'B';
	/* Read Ops. Trigger PageFault */
	printf("%#lx => %c\n", (unsigned long)base, *(char *)base);

	sleep(-1); /* Just for Debug */
	munmap(base, HPAGE_SIZE);

	return 0;
}
