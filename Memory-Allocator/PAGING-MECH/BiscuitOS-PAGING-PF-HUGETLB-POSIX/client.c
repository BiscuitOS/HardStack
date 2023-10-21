// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault - Hugetlb Memory on POSIX
 *
 * (C) 2023.10.18 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#define MAP_SIZE	(2 * 1024 * 1024)
#define MAP_VADDR	(0x6000000000)
#define SHMEM_FILE	"/mnt/BiscuitOS-Hugetlbfs/BiscuitOS.mem"

int main()
{
	char *base;
	int fd;

	fd = open(SHMEM_FILE, O_RDWR, 0777);
	if (fd < 0) {
		printf("Open %s failed.\n", SHMEM_FILE);
		return -1;
	}

	/* Alloc SHMEM */
	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd, 0);
	if (base == MAP_FAILED) {
		printf("MMAP POSIX failed.\n");
		return -1;
	}

	/* Write Ops, Trigger #PF */
	*base = 'H';

	/* Read from poxis share memory area, Don't Trigger #PF */
	printf("POSIX-Client: %#lx => %s\n", (unsigned long)base, base);

	/* unmap */
	munmap(base, MAP_SIZE);
	close(fd);

	return 0;
}
