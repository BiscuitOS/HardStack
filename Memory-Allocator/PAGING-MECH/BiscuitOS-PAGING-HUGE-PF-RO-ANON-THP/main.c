// SPDX-License-Identifier: GPL-2.0
/*
 * Huge-PageFault - RO Anonymous THP(HUGE Zero Page)
 *
 * (C) 2023.10.15 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(2 * 1024 * 1024)
#define MAP_VADDR	(0x6000000000)

int main()
{
	char *base, ch;

	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Read Ops, Trigger #PF */
	ch = *base;
	printf("THP ANON %#lx => %c\n", (unsigned long)base, ch);

	sleep(1); /* Just for Debug */
	/* Write Ops, Trigger #PF and SegmentFault */
	*base = 'D';

	munmap(base, MAP_SIZE);

	return 0;
}
