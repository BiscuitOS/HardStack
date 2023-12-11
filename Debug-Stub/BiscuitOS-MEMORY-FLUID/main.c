// SPDX-License-Identifier: GPL-2.0
/*
 * MEMORY FLUID
 *
 * (C) 2023.11.30 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)

int main()
{
	char *base;

	/* ALLOC VIRTUAL MEMORY */
	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* WRITE Ops, Trigger #PF */
	*base = 'B';
	/* READ Ops, Don't Trigger #PF */
	printf("Anonymous %#lx => %c\n", (unsigned long)base, *base);

	/* RECLAIM */
	munmap(base, MAP_SIZE);

	return 0;
}
