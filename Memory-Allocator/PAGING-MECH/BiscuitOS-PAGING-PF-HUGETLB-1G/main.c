// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault - Hugetlb Memory on 1Gig
 *
 *   CMDLINE="hugepagesz=1G hugepages=1"
 *
 * (C) 2023.10.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(1 * 1024 * 1024 * 1024)
#define MAP_VADDR	(0x6000000000)
#ifndef MAP_HUGE_1GB
#define HUGETLB_FLAG_ENCODE_SHIFT	26
#define MAP_HUGE_1GB	(30 << HUGETLB_FLAG_ENCODE_SHIFT)
#endif

int main()
{
	char *base;

	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS |
		    MAP_HUGETLB | MAP_HUGE_1GB,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Write Ops, Trigger #PF */
	*base = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("HUGETLB-1G %#lx => %c\n", (unsigned long)base, *base);

	munmap(base, MAP_SIZE);

	return 0;
}
