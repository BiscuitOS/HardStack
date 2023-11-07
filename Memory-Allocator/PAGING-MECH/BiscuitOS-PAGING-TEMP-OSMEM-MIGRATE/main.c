// SPDX-License-Identifier: GPL-2.0
/*
 * TEMPORARY MAPPING: OSMEM MIGRATE
 *
 * (C) 2023.11.04 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/* NUMA header with -lnuma */
#include <numa.h>
#include <numaif.h>
#include <sys/mman.h>

#define PAGE_SIZE	0x1000
#define NUMA_NODE0	0
#define NUMA_NODE1	1
#define VADDR_BASE	(0x6000000000)

int main()
{
	struct bitmask *old_nodes;
	struct bitmask *new_nodes;
	int status, nodes;
	char *base;

	/* NODEMASK ALLOC */
	old_nodes = numa_bitmask_alloc(numa_max_node() + 1);
	new_nodes = numa_bitmask_alloc(numa_max_node() + 1);

	/* PREALLOC MEMORY */
	base = mmap((void *)VADDR_BASE, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("No Free Memory for base\n");
		exit (-1);
	}

	/* VERIFY SRC NUMA NODE */
	numa_move_pages(0, 1, (void **)&base, NULL, &status, 0);
	printf("MIGRATE BEFORE: VADDR: %#lx NODE: %d\n",
					(unsigned long)base, status);

	/* SETUP DEST NUMA NODE */
	nodes = status == NUMA_NODE0 ? NUMA_NODE1 : NUMA_NODE0;
	numa_bitmask_setbit(old_nodes, status);
	numa_bitmask_setbit(new_nodes, nodes);
	status = nodes;

	/* MOVE TO ANOTHER NUMA NODE */
	numa_move_pages(0, 1, (void **)&base, &nodes, &status, 0);

	/* MIGRATING */
	if (numa_migrate_pages(0, old_nodes, new_nodes) < 0)
		printf("ERROR: numa_migrate_pages failed\n");

	/* CHECK DEST NUMA NODE */
	numa_move_pages(0, 1, (void **)&base, NULL, &status, 0);
	printf("MIGRATE AFTER:  VADDR: %#lx NODE: %d\n", 
					(unsigned long)base, status);
	
	munmap(base, PAGE_SIZE);
	numa_bitmask_free(new_nodes);
	numa_bitmask_free(old_nodes);

	return 0;
}
