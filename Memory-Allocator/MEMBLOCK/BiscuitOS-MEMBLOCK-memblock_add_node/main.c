/*
 * MEMBLOCK Memory Allocator: memblock_add_node
 *
 * (C) 2022.10.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memblock.h>

/* BiscuitOS NUMA Layout
 *  - Layout0: 2CPUs with 2 NUMA NODE
 *    NODE0: 0x00000000 - 0x100000000
 *    NODE1: 0x10000000 - 0x200000000
 *
 *  - Layout1: 4CPUs with 3 NUMA NODE
 *    NODE0: 0x00000000 - 0x200000000
 *    NODE1: 0x20000000 - 0x300000000
 *    NODE2: 0x30000000 - 0x400000000
 */
#define MEMBLOCK_FAKE_BASE	0x800000000
#define MEMBLOCK_FAKE_SIZE	0x100000
#define MEMBLOCK_FAKE_NODE	1

int __init BiscuitOS_Running(void)
{
	phys_addr_t region_start, region_end;
	u64 idx;
	int nid;

	/* Add new range */
	memblock_add_node(MEMBLOCK_FAKE_BASE, MEMBLOCK_FAKE_SIZE,
					MEMBLOCK_FAKE_NODE, MEMBLOCK_NONE);

	__for_each_mem_range(idx, &memblock.memory, NULL, MEMBLOCK_FAKE_NODE,
				MEMBLOCK_NONE, &region_start, &region_end, &nid)
		printk("Region %llx: %#llx - %#llx NID %d\n", idx,
					region_start, region_end, nid);

	/* Only Test: Remove range */
	memblock_remove(MEMBLOCK_FAKE_BASE, MEMBLOCK_FAKE_SIZE);

	return 0;
}
