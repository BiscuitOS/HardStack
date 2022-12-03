/*
 * MEMBLOCK Memory Allocator: Region Merge and Split
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

#define MEMBLOCK_FAKE_SIZE	0x100000
#define MEMBLOCK_FAKE_REGION0	0x810000000
#define MEMBLOCK_FAKE_REGION1	0x800000000
#define MEMBLOCK_FAKE_REGION2	0x820000000

int __init BiscuitOS_Running(void)
{
	phys_addr_t region_start, region_end;
	u64 idx;

	/* Add new range */
	memblock_add(MEMBLOCK_FAKE_REGION0, MEMBLOCK_FAKE_SIZE);

	/* Iterate membloc.memory */
	printk("=== Default Region ===\n");
	for_each_mem_range(idx, &region_start, &region_end)
		printk("Region %#llx: %#llx - %#llx\n", idx,
					region_start, region_end);

	/* Add Separation Region */
	memblock_add(MEMBLOCK_FAKE_REGION1, MEMBLOCK_FAKE_SIZE);
	memblock_add(MEMBLOCK_FAKE_REGION2, MEMBLOCK_FAKE_SIZE);

	/* Iterate membloc.memory */
	printk("=== ADD Region ===\n");
	for_each_mem_range(idx, &region_start, &region_end)
		printk("Region %#llx: %#llx - %#llx\n", idx,
					region_start, region_end);

	/* Only Test: Remove range */
	memblock_remove(MEMBLOCK_FAKE_REGION0, MEMBLOCK_FAKE_SIZE);
	memblock_remove(MEMBLOCK_FAKE_REGION1, MEMBLOCK_FAKE_SIZE);
	memblock_remove(MEMBLOCK_FAKE_REGION2, MEMBLOCK_FAKE_SIZE);

	return 0;
}
