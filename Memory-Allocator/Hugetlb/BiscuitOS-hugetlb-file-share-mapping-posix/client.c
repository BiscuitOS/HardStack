/*
 * Hugetlb: SYS V SHMEM Client on Shared File Hugepage!
 *
 * (C) 2021.11.29 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#define HPAGE_SIZE	(2UL * 1024 * 1024)
#define BISCUITOS_SIZE	(2 * HPAGE_SIZE)

int main()
{
	char *base;
	int fd;

	fd = shm_open("BiscuitOS", O_RDWR, 0777);
	if (fd < 0) {
		printf("Open /dev/shm/BiscuitOS failed.\n");
		return -1;
	}

	base = mmap(NULL, BISCUITOS_SIZE,
		    PROT_READ,
		    MAP_SHARED,
		    fd, 0);
	if (base == MAP_FAILED) {
		printf("MMAP POSIX failed.\n");
		return -1;
	}

	/* Read from poxis share memory area */
	printf("POSIX-Client: %s\n", base);

	/* unmap */
	munmap(base, BISCUITOS_SIZE);

	/* remove link */
	shm_unlink("BiscuitOS");

	return 0;
}
