// SPDX-License-Identifier: GPL-2.0
/*
 * SMAPS with SWAP-OUT Memory
 *
 * (C) 2023.08.25 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define HPAGE_SIZE	(4096 * 4)
#define MAP_VADDR	(0x6000000000)

int main()
{
	char *base;

	base = mmap((void *)MAP_VADDR, HPAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS |
		    MAP_FIXED_NOREPLACE,
		    -1, 0);
	if (base == MAP_FAILED) {
		printf("Alloc Hugetlb page failed.\n");
		return -1;
	}

	/* Write Ops */
	*(char *)base = 'B';
	/* Read Ops */
	printf("%#lx => %c\n", (unsigned long)base, *(char *)base);

	madvise(base, HPAGE_SIZE, MADV_PAGEOUT);

	sleep(-1); /* Just for Debug */
	munmap(base, HPAGE_SIZE);

	return 0;
}
