// SPDX-License-Identifier: GPL-2.0
/*
 * Paging Mechanism
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#define PAGE_SIZE	4096

int main()
{
	void *base;

	base = mmap(NULL, PAGE_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_SHARED | MAP_ANONYMOUS,
		   -1,
		   0);
	if (base == MAP_FAILED) {
		printf("Error: Virtual memory alloc failed.\n");
		return -1;
	}

	/* WRITE ops and Trigger PageFault */
	*(char *)base = 'B';
	/* READ ops */
	printf("%#lx => %c\n", (unsigned long)base, *(char *)base);

	munmap(base, PAGE_SIZE);

	return 0;
}
