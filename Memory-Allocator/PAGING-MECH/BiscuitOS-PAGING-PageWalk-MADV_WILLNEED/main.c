// SPDX-License-Identifier: GPL-2.0
/*
 * PageWalk: MADV_WILLNEED
 *
 * (C) 2023.08.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#define BISCUITOS_MAP_SIZE	4096

int main()
{
	unsigned long *val;
	char *base;

	/* mmap */
	base = (char *)mmap(NULL, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS,
			    -1,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -ENOMEM;
	}

	/* madvise MADV_WILLNEED, Trigger PageWalk */
	madvise(base, BISCUITOS_MAP_SIZE, MADV_WILLNEED);

	val = (unsigned long *)base;
	*val = 0x88520;
	printf("Default Value: %#lx\n", *val);

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);

	return 0;
}
