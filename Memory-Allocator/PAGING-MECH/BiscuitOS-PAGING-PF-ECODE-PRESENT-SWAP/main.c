// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault ERROR CODE: PF_PRESENT with SWAP
 *
 * (C) 2023.09.06 BuddyZhang1 <buddy.zhang@aliyun.com>
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

	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Write Ops, Trigger #PF Forbidden ZERO Page */
	*base = 'B';

	/* SWAP OUT */
	madvise(base, MAP_SIZE, MADV_PAGEOUT);
	sleep(1);

	/* SWAP IN, Trigger #PF with PF_PRESENT */
	*base = 'C';
	/* Read Ops, Don't Trigger #PF */
	printf("Anonymous %#lx => %c\n", (unsigned long)base, *base);

	munmap(base, MAP_SIZE);

	return 0;
}
