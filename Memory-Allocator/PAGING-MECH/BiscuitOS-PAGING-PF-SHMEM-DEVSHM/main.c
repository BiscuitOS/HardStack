// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Shmem Memory on DEV-SHM
 *
 * (C) 2023.09.22 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE		(4096)
#define MAP_VADDR		(0x6000000000)
#define SHAREDFILE_PATH		"/dev/shm/BiscuitOS.mem"

int main()
{
	char *base;
	int fd;

	/* Open SHM File and Don't Create */
	fd = open(SHAREDFILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: failed open %s\n", SHAREDFILE_PATH);
		return -EINVAL;
	}

	/* Alloc Shmem Memory */
	base = (char *)mmap((void *)MAP_VADDR, 
			    MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED,
			    fd,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -ENOMEM;
	}

	/* Write Ops, Trigger #PF */
	*base = 'H';
	/* Read Ops, Don't Trigger #PF */
	printf("DEV-SHM %#lx => %s\n", (unsigned long)base, base);
	
	sleep(-1); /* Just for Debug */
	/* unmap */
	munmap(base, MAP_SIZE);
	close(fd);

	return 0;
}
