// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: VMA VM_SYNC
 *
 *   CMDLINE ADD "memmap=16M!128M"
 *
 * (C) 2023.12.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_VADDR	(0x6000000000)
#define MAP_SIZE	(4 * 1024 * 1024) /* 4MiB */
#define FILE_PATH	"/mnt/DAX/BiscuitOS.txt"

int main()
{
	void *mem;
	int fd;

	/* OPEN FILE */
	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0)
		exit(-1);

	/* LAZYALLOC DAX MEMORY */
	mem = mmap((void *)MAP_VADDR, MAP_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_SYNC, /* MAP_SYNC TO VM_SYNC */
		   fd,
		   0);
	if (mem == MAP_FAILED)
		exit(-1);

	/* ACCESS */
	*(char *)mem = 'B'; /* Write Ops Trigger #PF */
	printf("MMAP: %#lx => %c\n", (unsigned long)mem, *(char *)mem);

	/* RECLAIM */
	munmap(mem, MAP_SIZE);
	close(fd);

	return 0;
}
