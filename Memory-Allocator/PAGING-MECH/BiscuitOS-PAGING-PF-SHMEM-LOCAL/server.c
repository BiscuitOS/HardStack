// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Shmem Memory on LOCAL
 *
 * (C) 2023.09.22 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#define MAP_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)
#define SHMEM_FILE	"/mnt/BiscuitOS-LOCAL-SHMEM/BiscuitOS.mem"

int main()
{
	char *base;
	int fd;

	/* Open SHMEM file and Don't Create File */
	fd = open(SHMEM_FILE, O_RDWR, 0777);
	if (fd < 0) {
		printf("POSIX open %s failed.\n", SHMEM_FILE);
		return -1;
	}

	/* Alloc Shared Memory */
	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd, 0);
	if (base == MAP_FAILED) {
		printf("POSIX mmap failed.\n");
		return -1;
	}

	/* Write Ops, Trigger #PF */
	*base = 'H';
	/* Read Ops, Don't Trigger #PF */
	printf("LOCAL-Server: %#lx => %s\n", (unsigned long)base, base);

	/* unmap */
	munmap(base, MAP_SIZE);
	close(fd);

	return 0;
}
