/*
 * Hugetlb: POSIX SHMEM Server on shared file hugepage
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

#define HPAGE_SIZE	(2 * 1024 * 1024)
#define BISCUITOS_SIZE	(2 * HPAGE_SIZE)
#define HUGETLBFS_FILE	"/mnt/BiscuitOS-Hugetlbfs/BiscuitOS"

int main()
{
	char *base;
	int fd;

	fd = open(HUGETLBFS_FILE, O_CREAT | O_RDWR, 0777);
	if (fd < 0) {
		printf("POSIX open %s failed.\n", HUGETLBFS_FILE);
		return -1;
	}

	/* MMAP */
	base = mmap(NULL, BISCUITOS_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd, 0);
	if (base == MAP_FAILED) {
		printf("POSIX mmap failed.\n");
		return -1;
	}

	/* Write to POSIX Share memory area */
	sprintf(base, "Hello %s", "BiscuitOS");
	printf("POSIX-Server: %s\n", base);

	/* unmap */
	munmap(base, BISCUITOS_SIZE);
	close(fd);

	return 0;
}
