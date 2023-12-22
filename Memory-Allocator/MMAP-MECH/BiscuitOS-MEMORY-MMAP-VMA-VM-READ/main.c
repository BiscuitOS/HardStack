// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: VMA VM_READ
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

	/* LAZYALLOC SHARED MEMORY */
	mem = mmap((void *)MAP_VADDR, MAP_SIZE,
		   PROT_READ, /* PROT_READ TO VM_READ */
		   MAP_SHARED | MAP_ANONYMOUS,
		   -1,
		   0);
	if (mem == MAP_FAILED)
		exit(-1);

	/* ACCESS */
	ch = *(char *)mem; /* READ Ops Trigger #PF */
	printf("MMAP: %#lx => %d\n", (unsigned long)mem, ch);

	/* RECLAIM */
	munmap(mem, MAP_SIZE);

	return 0;
}
