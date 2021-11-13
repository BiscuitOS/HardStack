/*
 * Hugetlb: Anonymous Private-mapping for 2MiB Hugepage
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

#define BISCUITOS_MAP_SIZE	(2 * 1024 * 1024)

int main()
{
	unsigned long *val;
	char *base;
	int pid;

	/* mmap */
	base = (char *)mmap(NULL, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
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

	/* Fork after write  */
	pid = fork();
	if (pid == 0) { /* Child Process */
		sleep(1); /* Write later */
		*val = 52088;
	} else { /* Parent Process */
		*val = 52188; /* Write first */
		sleep(1); 
	}

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);

	return 0;
}
