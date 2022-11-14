/*
 * MEMBLOCK Memory Allocator: __for_each_mem_range_rev
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

#define MEMBLOCK_FAKE_MEMORY	0x200000000
#define MEMBLOCK_FAKE_RESERVE	0x300000000
#define MEMBLOCK_FAKE_SIZE	0x1000
#define MEMBLOCK_FAKE_NODE	0

int __init BiscuitOS_Running(void)
{
	phys_addr_t region_start, region_end;
	u64 idx;
	int nid;

	/* Add Fake Regions */
	memblock_add(MEMBLOCK_FAKE_MEMORY, MEMBLOCK_FAKE_SIZE);
	memblock_reserve(MEMBLOCK_FAKE_RESERVE, MEMBLOCK_FAKE_SIZE);

	/* Itereate over reserve regions on speical NUMA NODE */
	printk("=== Iterator Reserved on NUMA NODE %d\n", MEMBLOCK_FAKE_NODE);
	__for_each_mem_range_rev(idx, &memblock.reserved, NULL,
				MEMBLOCK_FAKE_NODE, MEMBLOCK_NONE,
				&region_start, &region_end, &nid)
		printk("REGION%llx NID %d: %#llx - %#llx\n", idx, nid,
						region_start, region_end);

	/* Itereate noinclude reserve regions on speical NUMA NODE */
	printk("=== Iterator No-include on NUMA NODE%d\n", MEMBLOCK_FAKE_NODE);
	__for_each_mem_range_rev(idx, &memblock.reserved, &memblock.memory,
				MEMBLOCK_FAKE_NODE, MEMBLOCK_NONE,
				&region_start, &region_end, &nid)
		printk("REGION%llx NID %d: %#llx - %#llx\n", idx, nid,
						region_start, region_end);
	/* Remove Fake Regions */
	memblock_remove(MEMBLOCK_FAKE_MEMORY,  MEMBLOCK_FAKE_SIZE);
	memblock_remove(MEMBLOCK_FAKE_RESERVE, MEMBLOCK_FAKE_SIZE);

	return 0;
}
