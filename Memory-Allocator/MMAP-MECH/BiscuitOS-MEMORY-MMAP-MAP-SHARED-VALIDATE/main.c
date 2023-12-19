// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: MAP_SHARED_VALIDATE
 *
 * (C) 2023.12.17 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_VADDR	(0x6000000000)
#define MAP_SIZE	(4096)
#define FILE_PATH	"/mnt/BiscuitOS.txt"
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main()
{
	void *mem;
	int fd;

	/* OPEN FILE */
	fd = open(FILE_PATH, O_RDONLY);
	if (fd < 0)
		errExit("OPEN FAILED.\n");

	/* LAZYALLOC VIRTUAL MEMORY */
	mem = mmap((void *)MAP_VADDR, MAP_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_SHARED_VALIDATE,
		   fd,
		   0);
	if (mem == MAP_FAILED)
		errExit("MMAP FAILED.\n");

	/* ACCESS */
	*(char *)mem = 'B'; /* Write Ops Trigger #PF */
	printf("MMAP: %#lx => %c\n", (unsigned long)mem, *(char *)mem);

	/* RECLAIM */
	munmap(mem, MAP_SIZE);

	return 0;
}
