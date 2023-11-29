// SPDX-License-Identifier: GPL-2.0
/*
 * PREALLOC: PFN-MAPPING Memory
 * 
 *   CMDLINE ADD "memmap=4K$0x10000000"
 *
 * (C) 2023.11.19 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FILE_PATH	"/dev/mem"
#define MAP_VADDR	(0x6000000000)
#define MAP_SIZE	(4096)
#define PFNMAP_BASE	0x10000000

int main()
{
	void *mem;
	int fd;

	/* OPEN FILE */
	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0)
		exit(-1);

	/* PREALLOC PFNMAP MEMORY */
	mem = mmap((void *)MAP_VADDR, MAP_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_SHARED,
		   fd,
		   PFNMAP_BASE);
	if (mem == MAP_FAILED)
		exit(-1);

	/* ACCESS */
	*(char *)mem = 'B'; /* Don't Trigger #PF */
	printf("PALLOC-PFN: %#lx => %c\n", (unsigned long)mem, *(char *)mem);

	/* RECLAIM */
	munmap(mem, MAP_SIZE);
	close(fd);

	return 0;
}
