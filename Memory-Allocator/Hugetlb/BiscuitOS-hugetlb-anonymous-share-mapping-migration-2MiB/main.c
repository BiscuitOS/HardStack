/*
 * Hugetlb: Shared Anonymous hugepage for migration
 *
 * (C) 2021.11.29 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifdef __i386__
#error "Process doesn't support I386 Architecture"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman.h>
/* NUMA header with -lnuma */
#include <numa.h>
#include <numaif.h>
/* errno */
#include <errno.h>

#define NUMA_NODE0			0
#define NUMA_NODE1			1
#define NUMA_AREA			2

#define HPAGE_SIZE			(2 * 1024 * 1024)
#define HPAGE_MASK			(~(HPAGE_SIZE - 1))
#define BISCUITOS_MAP_SIZE		(2 * HPAGE_SIZE)

#ifndef MAP_HUGE_2MB
#define HUGETLB_FLAG_ENCODE_SHIFT	26
#define MAP_HUGE_2MB			(21 << HUGETLB_FLAG_ENCODE_SHIFT)
#endif

int main()
{
	struct bitmask *old_nodes;
	struct bitmask *new_nodes;
	void *addr[NUMA_AREA];
	char *page_base;
	int nr_nodes;
	char *pages;
	int status;
	int nodes;
	int rc;

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

	page_base = (char *)mmap(NULL,
				BISCUITOS_MAP_SIZE,
				PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS |
				MAP_HUGETLB | MAP_HUGE_2MB,
				-1,
				0);
	if (page_base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -ENOMEM;
	}

	/* Page Alignment: Bound to HugePage */
	pages = (void *)((((long)page_base) & HPAGE_MASK) + HPAGE_SIZE);

	/* Prepare: Bind to Physical Page */
	pages[0] = 0;
	addr[0] = pages;

	/* Verify correct startup locations */
	printf("BiscuitOS: Before Migration\n");
	numa_move_pages(0, 1, addr, NULL, &status, 0);
	printf("  Page vaddr: %p node: %d\n", pages, status);

	/* sleep just for debug */
	sleep(20);

	/* Move to another NUMA NODE */
	nodes = status == NUMA_NODE0 ? NUMA_NODE1 : NUMA_NODE0;
	numa_bitmask_setbit(old_nodes, status);
	numa_bitmask_setbit(new_nodes, nodes);
	status = nodes;

	/* Move to another NUMA NODE */
	numa_move_pages(0, 1, addr, &nodes, &status, 0);

	/* Migration */
	printf("Migrating the current processes pages ...\n");
	rc = numa_migrate_pages(0, old_nodes, new_nodes);
	if (rc < 0)
		printf("ERROR: numa_migrate_pages failed\n");

	/* Get page state after migration */
	numa_move_pages(0, 1, addr, NULL, &status, 0);
	printf("  Page vaddr: %p node: %d\n", pages, status);
	
	/* sleep just for debug */
	sleep(20);

	/* unmap */
	munmap(page_base, BISCUITOS_MAP_SIZE);
err_node:
	numa_bitmask_free(new_nodes);
	numa_bitmask_free(old_nodes);
	return rc;
}
