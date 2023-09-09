// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Read-Only Shmem
 *
 * (C) 2023.09.01 BuddyZhang1 <buddy.zhang@aliyun.com>
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

	base = mmap((void *)MAP_VADDR, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Read Ops, Trigger #PF */
	ch = *base;
	/* Read Ops, Don't Trigger #PF */
	printf("%#lx => %c\n", (unsigned long)base, ch);

	munmap(base, PAGE_SIZE);

	return 0;
}
