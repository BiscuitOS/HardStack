// SPDX-License-Identifier: GPL-2.0
/*
 * MEMMAP: NUMA NODE Information
 *
 * (C) 2023.06.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <asm/unistd_64.h>

#define PAGE_SIZE	4096

int main()
{
	void *base;
	int node;

	/* Alloc Memory */
	base = mmap(NULL, PAGE_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE,
			-1,
			0);
	if (base == MAP_FAILED) {
		printf("ERROR: No Free Memory\n");
		exit (1);
	}

	/* INFO: NUMA NODE */
	/* mov_page from libnuma, not sys_move_pages */
	syscall(__NR_move_pages, 0, 1, &base, NULL, &node, 0);
	printf("Virtual-Address on NUM Node %d\n", node);

	munmap(base, PAGE_SIZE);

	return 0;
}
