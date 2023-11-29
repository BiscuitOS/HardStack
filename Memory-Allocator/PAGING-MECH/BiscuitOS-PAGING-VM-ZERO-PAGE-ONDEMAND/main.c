// SPDX-License-Identifier: GPL-2.0
/*
 * ZERO PAGE: ONDEMAND
 *
 * (C) 2023.11.21 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)

int main()
{
	char *base, ch;

	/* ALLOC ANONYMOUS MEMORY */
	base = mmap((void *)MAP_VADDR, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* ONDEMAND: MAPPING ZERO PAGE */
	madvise(base, PAGE_SIZE, 22 /* MADV_POPULATE_READ */);

	/* READ Ops, Don't Trigger #PF */
	ch = *base;
	/* READ Ops, Don't Trigger #PF */
	printf("ZERO-PAGE-OD: %#lx => %d\n", (unsigned long)base, ch);

	/* RECLAIM */
	munmap(base, PAGE_SIZE);

	return 0;
}
