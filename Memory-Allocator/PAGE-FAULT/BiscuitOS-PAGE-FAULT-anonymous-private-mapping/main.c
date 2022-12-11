/*
 * PageFault Mechanism: anonymous private mapping
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.10.16 BiscuitOS
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

#define BISCUITOS_MAP_SIZE	4096

int main()
{
	char *base;

	/* alloc virtual address */
	base = (char *)mmap(NULL, BISCUITOS_MAP_SIZE,
				PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS,
				-1, 0);
	if (base == MAP_FAILED) {
		printf("Uncorrect Error: No free Virtual memory\n");
		return -1;
	}

	/* Trigger page fault */
	*(char *)base = 'B';

	munmap(base, BISCUITOS_MAP_SIZE);
	return 0;
}
