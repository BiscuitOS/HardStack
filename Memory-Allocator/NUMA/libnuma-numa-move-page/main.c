/*
 * NUMA: Migrate Page to another NUMA NODE
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

#define NUMA_NODE0	0
#define NUMA_NODE1	1
#define NUMA_AREA	2

int main()
{
	struct bitmask *old_nodes;
	struct bitmask *new_nodes;
	unsigned int pagesize;
	void *addr[NUMA_AREA];
	char *page_base;
	int nr_nodes;
	char *pages;
	int status;
	int nodes;
	int rc;

	/* Default Page Size */
	pagesize = getpagesize();
	/* NUMA NODE */
	nr_nodes = numa_max_node() + 1;

	/* NODEMASK alloc */
	old_nodes = numa_bitmask_alloc(nr_nodes);
	new_nodes = numa_bitmask_alloc(nr_nodes);

	if (nr_nodes < 2) {
		printf("Request minimum of 2 nodes!\n");
		rc = -EINVAL;
		goto err_node;
	}

	/* Prepare Vritual address */
	page_base = malloc((pagesize + 1));
	if (!page_base) {
		printf("No Free Memory for page_base\n");
		rc = -ENOMEM;
		goto err_node;
	}

	/* Page Alignment: Bound to Page */
	pages = (void *)((((long)page_base) & ~((long)(pagesize - 1))) + 
								pagesize);
	/* Prepare: Bind to Physical Page */
	pages[0] = 0;
	addr[0] = pages;

	/* Verify correct startup locations */
	printf("BiscuitOS: Before Migration\n");
	numa_move_pages(0, 1, addr, NULL, &status, 0);
	printf("  Page vaddr: %p node: %d\n", pages, status);

	/* Move to another NUMA NODE */
	nodes = status == NUMA_NODE0 ? NUMA_NODE1 : NUMA_NODE0;
	numa_bitmask_setbit(old_nodes, status);
	numa_bitmask_setbit(new_nodes, nodes);
	status = nodes;

	/* Move to another NUMA NODE */
	syscall(600, 1);
	numa_move_pages(0, 1, addr, &nodes, &status, 0);
	syscall(600, 0);

	/* Migration */
	printf("Migrating the current processes pages ...\n");
	rc = numa_migrate_pages(0, old_nodes, new_nodes);
	if (rc < 0)
		printf("ERROR: numa_migrate_pages failed\n");

	/* Get page state after migration */
	numa_move_pages(0, 1, addr, NULL, &status, 0);
	printf("  Page vaddr: %p node: %d\n", pages, status);
	
	free(page_base);
err_node:
	numa_bitmask_free(new_nodes);
	numa_bitmask_free(old_nodes);
	return rc;
}
