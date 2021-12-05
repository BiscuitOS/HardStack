/*
 * Hugetlb: Anonymous Shared-mapping for 64K Hugepage
 *
 * (C) 2021.12.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __aarch64__
#error "Process Need running on ARM64 Architecture"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman.h>

#define HPAGE_SIZE		(64 * 1024)
#define BISCUITOS_MAP_SIZE	(2 * HPAGE_SIZE)

#ifdef __aarch64__
#define HUGETLB_FLAG_ENCODE_SHIFT	26
#define MAP_HUGE_64KB			(16 << HUGETLB_FLAG_ENCODE_SHIFT)
#endif

int main()
{
	char *base;

	/* mmap */
	base = (char *)mmap(NULL, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED | MAP_ANONYMOUS | 
			    MAP_HUGETLB | MAP_HUGE_64KB,
			    -1,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -ENOMEM;
	}

	/* Trigger page fault */
	base[0] = 'B';
	printf("%#lx => %c\n", (unsigned long)base, base[0]);

	/* Just for debug */
	sleep(-1);

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);

	return 0;
}
