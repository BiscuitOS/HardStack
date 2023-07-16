// SPDX-License-Identifier: GPL-2.0
/*
 * TLB: FLUSH TLB on madvise(1)
 *
 * (C) 2023.07.15 BuddyZhang1 <buddy.zhang@aliyun.com>
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
	void *base;

	/* mmap */
	base = mmap(NULL, BISCUITOS_MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -ENOMEM;
	}

	sprintf((char *)base, "Hello BiscuitOS");
	printf("Default Value: %s %#lx\n", (char *)base, (unsigned long)base);

	/* madvise: Free PageTable and Trigger TLB Flush */
	madvise(base, BISCUITOS_MAP_SIZE, MADV_FREE);

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);

	return 0;
}
