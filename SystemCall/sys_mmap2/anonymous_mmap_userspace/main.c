/*
 * Anonymous mmap from Userspace on BiscuitOS
 *
 * (C) 2020.10.24 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

/* The size for anonymous mmap */
#define MMAP_SIZE		4096

int main()
{
	char *base;

	/* Anonymous mmap */
	base = mmap(NULL,
		    MMAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS,
		    -1,
		    0);
	if (!base) {
		printf("ERROR: Mmap anonymous failed.\n");
		return -1;
	}

	/* Use */
	sprintf(base, "BiscuitOS");
	printf("=> %s [%#lx]\n", base, (unsigned long)base);

	/* Un-mmap */
	munmap(base, MMAP_SIZE);

	return 0;
}
