/*
 * PreAlloc Memory with MAP_LOCKED
 *
 * (C) 2023.02.14 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
	void *addr;

	/* mmap: prealloc */
	addr = mmap(NULL, 
		    BISCUITOS_MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS |
		    MAP_LOCKED,
		    -1,
		    0);
	if (addr == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -ENOMEM;
	}

	*(char *)addr = 'B';
	printf("%#lx => %c\n", (unsigned long)addr, *(char *)addr);

	/* unmap */
	munmap(addr, BISCUITOS_MAP_SIZE);

	return 0;
}
