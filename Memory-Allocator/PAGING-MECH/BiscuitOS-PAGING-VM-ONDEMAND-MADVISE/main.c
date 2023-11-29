// SPDX-License-Identifier: GPL-2.0
/*
 * ONDEMAND: madvise
 *
 * (C) 2023.11.20 BuddyZhang1 <buddy.zhang@aliyun.com>
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

	/* ALLOC ANONYMOUS MEMORY */
	mem = mmap((void *)MAP_VADDR, MAP_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS,
		   -1,
		   0);
	if (mem == MAP_FAILED)
		exit(-1);

	/* ONDEMAND */
	madvise(mem, MAP_SIZE, 23 /* MADV_POPULATE_WRITE */);

	/* ACCESS */
	*(char *)mem = 'B'; /* Don't Trigger #PF */
	printf("OD-MADVISE: %#lx => %c\n", (unsigned long)mem, *(char *)mem);

	/* RECLAIM */
	munmap(mem, MAP_SIZE);

	return 0;
}
