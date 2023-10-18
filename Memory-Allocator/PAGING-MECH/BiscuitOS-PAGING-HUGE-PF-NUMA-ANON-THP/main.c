// SPDX-License-Identifier: GPL-2.0
/*
 * Huge-PageFault - Anonymous THP on NUMA Balancing
 *
 *  Enable CONFIG_NUMA_BALANCING and CMDLINE='numa_balancing=disable'
 *
 * (C) 2023.10.15 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
/* NUMA header with -lnuma */
#include <numa.h>
#include <numaif.h>

#define MAP_SIZE	(2 * 1024 * 1024)
#define MAP_VADDR	(0x6000000000)
#define NUMA_NODE0	0
#define NUMA_NODE1	1

int main()
{
	struct bitmask *old, *new;
	int status, nodes;
	char *base;

	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}
	/* Write Ops, Trigger #PF */
	*base = 'B';

	/* Get Current NUMA NODE Information */
	numa_move_pages(0, 1, (void **)&base, NULL, &status, 0);
	old = numa_bitmask_alloc(numa_max_node() + 1);
	new = numa_bitmask_alloc(numa_max_node() + 1);
	nodes = status == NUMA_NODE0 ? NUMA_NODE1 : NUMA_NODE0;
	numa_bitmask_setbit(old, status);
	numa_bitmask_setbit(new, nodes);
	printf("%d MOVE Page From NUMA NODE%d To NUMA NODE%d\n", getpid(), status, nodes);
	status = nodes;
	
	/* Migrate Page */
	numa_move_pages(0, 1, (void **)&base, &nodes, &status, 0);
	numa_migrate_pages(0, old, new);

	/* Write Ops, Trigger #PF with NUMA */
	*base = 'D';
	
	sleep(-1);
	munmap(base, MAP_SIZE);

	return 0;
}
