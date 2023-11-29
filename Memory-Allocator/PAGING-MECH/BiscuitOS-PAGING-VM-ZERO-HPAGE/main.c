// SPDX-License-Identifier: GPL-2.0
/*
 * ZERO HUGE PAGE
 *
 * (C) 2023.11.21 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE	(2 * 1024 * 1024) /* 2MiB */
#define MAP_VADDR	(0x6000000000)

int main()
{
	char *base, ch;

	/* ALLOC ANONYMOUS THP MEMORY */
	base = mmap((void *)MAP_VADDR, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* READ Ops, Trigger #PF MAPPING ZERO HUGE PAGE */
	ch = *base;
	/* READ Ops, Don't Trigger #PF */
	printf("ZERO-HPAGE: %#lx => %d\n", (unsigned long)base, ch);

	/* RECLAIM */
	munmap(base, PAGE_SIZE);

	return 0;
}
