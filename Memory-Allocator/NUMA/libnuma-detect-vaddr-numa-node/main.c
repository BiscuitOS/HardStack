/*
 * NUMA: Detect NUMA NODE information for Virtual Address.
 *
 * (C) 2021.04.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/* NUMA header with -lnuma */
#include <numa.h>
#include <numaif.h>
/* errno */
#include <errno.h>

int main()
{
	unsigned int pagesize;
	char *page_base;
	char *pages;
	void *addr[2];
	int status;

	/* Default Page Size */
	pagesize = getpagesize();

	page_base = malloc((pagesize + 1));
	if (!page_base) {
		printf("No Free Memory for page_base\n");
		return -ENOMEM;
	}

	/* Bound to page */
	pages = (void *)((((long)page_base) & ~((long)(pagesize - 1))) + 
								pagesize);
	/* Prepare */
	addr[0] = pages;

	numa_move_pages(0, 1, addr, NULL, &status, 0);
	printf("Page vaddr=%p node=%d\n", pages, status);

	free(page_base);
	return 0;
}
