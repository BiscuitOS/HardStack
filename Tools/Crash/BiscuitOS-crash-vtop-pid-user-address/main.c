/*
 * CRASH: vtop -c pid 
 *
 * (C) 2021.06.06 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#define VADDR			0x8000000000

int main()
{
	char *base;

	/* mmap */
	base = (char *)mmap(VADDR, BISCUITOS_MAP_SIZE, PROT_READ | PROT_WRITE,
			    MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -ENOMEM;
	}

	/* Trigger page-fault */
	base[0] = 'A';
	printf("%#lx => %c\n", (unsigned long)base, base[0]);

	/* Loop for CRASH */
	while (1);

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);

	return 0;
}
