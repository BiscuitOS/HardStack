// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Anonymous Memory on Heap
 *
 * (C) 2023.09.17 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(4096)

int main()
{
	char *base, *heap;

	/* Obtain Heap End Address */
	base = sbrk(0); 

	/* Alloc Memroy From Heap */
	heap = base + MAP_SIZE;
	if (brk((void *)heap) != 0) {
		printf("ERROR Brk failed.\n");
		exit (1);
	}
	/* Memory Range */
	printf("Brk Range: %#lx - %#lx\n", (unsigned long)base, 
					   (unsigned long)sbrk(0));

	/* Write Ops, Trigger #PF with Heap */
	*base = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("Heap %#lx => %c\n", (unsigned long)base, *base);

	/* Reclaim Heap Memory */
	brk(base);

	return 0;
}
