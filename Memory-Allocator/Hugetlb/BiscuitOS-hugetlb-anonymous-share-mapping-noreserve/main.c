/*
 * Hugetlb: Anonymous Shared-mapping for NO Reserved using Hugepage
 *
 * (C) 2021.11.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman.h>

#define HPAGE_SIZE		(2 * 1024 * 1024)
#define BISCUITOS_MAP_SIZE	(2 * HPAGE_SIZE)

int main()
{
	char *base;

	/* mmap */
	base = (char *)mmap(NULL, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED | MAP_ANONYMOUS | 
			    MAP_HUGETLB | MAP_NORESERVE,
			    -1,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -ENOMEM;
	}

	/* Trigger page fault */
	base[0] = 'B';
	printf("%#lx => %c\n", (unsigned long)base, base[0]);

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);

	return 0;
}
