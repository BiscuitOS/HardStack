// SPDX-License-Identifier: GPL-2.0-only
/*
 * PageFault with Anonymous HugeTLB
 *
 * (C) 2023.09.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(2 * 1024 * 1024)

int main()
{
	char *base;

	/* mmap */
	base = (char *)mmap(NULL, MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS |
			    MAP_HUGETLB,
			    -1,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -ENOMEM;
	}

	/* Write Ops, Trigger #PF */
	*base = 'B';
	/* Read Ops, Don't trigger #PF */
	printf("%#lx => %c\n", (unsigned long)base, *base);

	/* unmap */
	munmap(base, MAP_SIZE);

	return 0;
}
