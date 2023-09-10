// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault
 *
 * (C) 2023.09.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <numa.h>
#include <numaif.h>
#include <sys/mman.h>

#define MAP_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)
#define NUMA_NODE0	0
#define NUMA_NODE1	1
#define NUMA_NR		2

int main()
{
	struct bitmask *old_nodes;
	struct bitmask *new_nodes;
	int status, nodes, rc;
	void *base;

	/* Alloc Anonymous page */
	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1, 0);
	if (base == MAP_FAILED) {
		printf("MMAP FAILED.\n");
		exit(-1);
	}
	sprintf((char *)base, "Hello BiscuitOS");
	printf("Migrate %#lx => %s\n", (unsigned long)base, (char *)base);

	/* Setup NUMA Bitmap */
	old_nodes = numa_bitmask_alloc(NUMA_NR + 1);
	new_nodes = numa_bitmask_alloc(NUMA_NR + 1);
	nodes = NUMA_NODE0;
	numa_bitmask_setbit(old_nodes, nodes);
	nodes = NUMA_NODE1;
	numa_bitmask_setbit(new_nodes, nodes);
	status = nodes;

	/* Migration */
	syscall(600, 1);
	numa_move_pages(0, 1, &base, &nodes, &status, 0);
	syscall(600, 0);
	rc = numa_migrate_pages(0, old_nodes, new_nodes);
	if (rc < 0)
		printf("ERROR: numa_migrate_pages failed\n");

	numa_bitmask_free(new_nodes);
	numa_bitmask_free(old_nodes);
	munmap(base, MAP_SIZE);

	return 0;
}
