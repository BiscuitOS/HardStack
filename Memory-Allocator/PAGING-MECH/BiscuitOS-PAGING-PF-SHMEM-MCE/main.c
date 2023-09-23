// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Shmem on MCE
 *
 * (C) 2023.09.22 BuddyZhang1 <buddy.zhang@aliyun.com>
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
		    MAP_SHARED | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Write Ops, Trigger #PF */
	*base = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("SHMEM-MCE %#lx => %c\n", (unsigned long)base, *base);

	/* MCE: UE */
	madvise(base, MAP_SIZE, MADV_HWPOISON);

	/* Write Ops, Trigger #PF with UE */
	*base = 'D';

	munmap(base, MAP_SIZE);

	return 0;
}
