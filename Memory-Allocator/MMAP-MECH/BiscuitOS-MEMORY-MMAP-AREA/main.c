// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP AREA: NO INTERSECTION AREA
 *
 * (C) 2023.12.19 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#define MAP_VADDR1	(0x6000000000)
#define MAP_VADDR2	(0x6000001000)
#define MAP_SIZE	(4096 * 2)

/* LAZY ALLOC ANONYMOUS MEMORY */
static void *alloc_memory(unsigned long addr, unsigned long size)
{
	return mmap((void *)addr, size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

int main()
{
	unsigned long mem1, mem2;

	/* ONLY ALLOC VIRTUAL MEMORY */
	mem1 = (unsigned long)alloc_memory(MAP_VADDR1, MAP_SIZE);
	mem2 = (unsigned long)alloc_memory(MAP_VADDR2, MAP_SIZE);

	printf("AREA1: %#lx - %#lx\n", mem1, mem1 + MAP_SIZE);
	printf("AREA2: %#lx - %#lx\n", mem2, mem2 + MAP_SIZE);
	sleep(-1); /* JUST FOR DEBUG */

	/* RECLAIM */
	munmap((void *)mem1, MAP_SIZE);
	munmap((void *)mem2, MAP_SIZE);

	return 0;
}
