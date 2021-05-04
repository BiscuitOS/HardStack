/*
 * SYS_mmap: MAP_FIXED/MAP_FIXED_NOREPLACE/MAP_FIXED_SAFE
 *
 * (C) 2021.05.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#ifdef __i386__ /* Intel i386 */
#define AREA_PREV_BASE		0x60000000
#define AREA_NEXT_BASE		0x60008000
#define AREA_BASE		0x60007000
#else           /* AMD x86_64 */
#define AREA_PREV_BASE		0x600000000000
#define AREA_NEXT_BASE		0x600000008000
#define AREA_BASE		0x600000007000
#endif
#define PAGE_SIZE		4096
#define MAP_FIXED_NOREPLACE	0x100000

static void *BiscuitOS_anonymous_mmap(unsigned long addr,
					unsigned long size, unsigned long flags)
{
	return mmap((void *)addr, size, PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS | flags, -1, 0);
}

static void BiscuitOS_anonymous_munmap(unsigned long addr, unsigned long size)
{
	munmap((void *)addr, size);
}

int main()
{
	char *base[3];

	/* mmap default region */
	base[0] = BiscuitOS_anonymous_mmap(AREA_PREV_BASE, 1 * PAGE_SIZE, 0);
	base[1] = BiscuitOS_anonymous_mmap(AREA_NEXT_BASE, 4 * PAGE_SIZE, 0);

	/* Trigger page fault */
	base[1][0] = 'B';
	base[1][PAGE_SIZE] = 'C';
	printf("Default: %#lx => %c\n", (unsigned long)base[1], base[1][0]);

	/* expand VMA */
	base[2] = BiscuitOS_anonymous_mmap(AREA_BASE, 2 * PAGE_SIZE, MAP_FIXED);
	if (base[2] == MAP_FAILED) {
		printf("MAP FAILED.\n");
		exit(1);
	}

	printf("Modify:  %#lx => %c\n", (unsigned long)base[1], base[1][0]);
	/* only for debug */
	printf("PID: %ld\n", (long)getpid());
	sleep(250);

	/* Unmapp */
	BiscuitOS_anonymous_munmap(AREA_BASE, 3 * PAGE_SIZE);
	BiscuitOS_anonymous_munmap(AREA_NEXT_BASE, 4 * PAGE_SIZE);
	BiscuitOS_anonymous_munmap(AREA_PREV_BASE, 1 * PAGE_SIZE);

	return 0;
}
