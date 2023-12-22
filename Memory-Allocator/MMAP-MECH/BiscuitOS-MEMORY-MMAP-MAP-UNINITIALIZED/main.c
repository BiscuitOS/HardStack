// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: MAP_UNINITIALIZED
 *
 * (C) 2023.12.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#define MAP_VADDR	(0x6000000000)
#define MAP_SIZE	(4096)

int main()
{
	void *mem;
	char ch;

	/* LAZYALLOC ANONYMOUS MEMORY */
	mem = mmap((void *)MAP_VADDR, MAP_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS |
		   0x4000000 /* MAP_UNINITIALIZED */,
		   -1,
		   0);
	if (mem == MAP_FAILED)
		exit(-1);

	/* ACCESS */
	ch = *(char *)mem; /* Read Ops Trigger #PF */
	printf("MMAP: %#lx => %d\n", (unsigned long)mem, ch);

	/* RECLAIM */
	munmap(mem, MAP_SIZE);

	return 0;
}
