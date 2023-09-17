// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Anonymous on RO(Read-Only Zero Page)
 *
 * (C) 2023.09.01 BuddyZhang1 <buddy.zhang@aliyun.com>
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

	/* Read Ops, Trigger #PF, and ZERO Page */
	ch = *base;
	/* Read Ops, Don't Trigger #PF */
	printf("ANON-RO %#lx => %d\n", (unsigned long)base, *base);

	munmap(base, MAP_SIZE);

	return 0;
}
