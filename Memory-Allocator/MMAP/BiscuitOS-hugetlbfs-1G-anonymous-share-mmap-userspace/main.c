/*
 * Anonymout Hugepage(Hugetlbfs) 1GiB share mmap on Userspace
 *
 * (C) 2021.04.02 BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define BISCUITOS_MAP_SIZE	(1 * 1024 * 1024 * 1024)

int main()
{
	unsigned long *val;
	char *base;

	/* mmap */
	base = (char *)mmap(NULL, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB,
			    -1,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -ENOMEM;
	}

	val = (unsigned long *)base;
	/* Trigger page fault */
	*val = 88520;
	printf("%#lx => %ld\n", (unsigned long)val, *val);

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);

	return 0;
}
