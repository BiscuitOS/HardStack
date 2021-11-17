/*
 * Hugetlb: Fork on File Privated-mapping 2MiB Hugepage
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

#define HPAGE_SIZE		(2 * 1024 * 1024)
#define BISCUITOS_MAP_SIZE	(2 * HPAGE_SIZE)
#define BISCUITOS_HUGEPAGE_PATH	"/mnt/BiscuitOS-hugetlbfs/hugepage"

int main()
{
	unsigned long *val;
	char *base;
	int fd, pid;

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
			    MAP_PRIVATE,
			    fd,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -ENOMEM;
	}

	val = (unsigned long *)base;
	/* Trigger page fault */
	*val = 88520;
	printf("%#lx => %ld\n", (unsigned long)val, *val);

	/* Fork after Write */
	pid = fork();
	if (pid == 0) { /* Child Process */
		sleep(1);
		*val = 52088; /* Write after */
	} else { /* Parent Process */
		*val = 88990; /* Write first */
		sleep(2);
	}

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);
	close(fd);

	return 0;
}
