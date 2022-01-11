/*
 * Hugeltb: File Shared-mapping for Hugepage
 *
 * (C) 2022.01.11 BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define HPAGE_SIZE		(2 * 1024 * 1024)
#define BISCUITOS_MAP_SIZE	(2 * HPAGE_SIZE)
#define BISCUITOS_HUGEPAGE_PATH	"/mnt/BiscuitOS-hugetlbfs/hugepage"

int main()
{
	char *base;
	int fd;

	/* Open Hugepage */
	fd = open(BISCUITOS_HUGEPAGE_PATH, O_RDWR | O_CREAT);
	if (fd < 0) {
		printf("ERROR: failed open %s\n", BISCUITOS_HUGEPAGE_PATH);
		return -EINVAL;
	}

	/* mmap */
	base = (char *)mmap(NULL, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED,
			    fd,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -ENOMEM;
	}

	/* Trigger page fault */
	base[0] = 'B';
	printf("%#lx => %c\n", (unsigned long)base, base[0]);
	/* Only debug */
	sleep(10);

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);
	close(fd);

	return 0;
}
