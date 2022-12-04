/*
 * KSM
 *
 * (C) 2021.08.02 BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define BISCUITOS_MAP_SIZE	(6 * 4096)
#define BISCUITOS_BASE0		0x60000000
#define BISCUITOS_BASE1		0x80000000
#define MAP_FLAGS		(MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED)

int main()
{
	unsigned long *val;
	char *base0, *base1;

	/* mmap */
	base0 = (char *)mmap(BISCUITOS_BASE0, BISCUITOS_MAP_SIZE,
			PROT_READ | PROT_WRITE, MAP_FLAGS, -1, 0);
	base1 = (char *)mmap(BISCUITOS_BASE1, BISCUITOS_MAP_SIZE,
			PROT_READ | PROT_WRITE, MAP_FLAGS, -1, 0);

	/* setup memory region as mergable */
	madvise(base0, BISCUITOS_MAP_SIZE, MADV_MERGEABLE);
	madvise(base1, BISCUITOS_MAP_SIZE, MADV_MERGEABLE);

	/* Trigger alloc page and build same content page */
	memset(base0, 'a', 4096 * 2);
	memset(base1, 'a', 4096 * 2);

	/* Wait to merge */
	sleep(10);

	/* setup memory region as un-mergable */
	madvise(base0, BISCUITOS_MAP_SIZE, MADV_UNMERGEABLE);
	madvise(base1, BISCUITOS_MAP_SIZE, MADV_UNMERGEABLE);

	/* unmap */
	munmap(base0, BISCUITOS_MAP_SIZE);
	munmap(base1, BISCUITOS_MAP_SIZE);

	return 0;
}
