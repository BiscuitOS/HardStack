/*
 * MEMBLOCK Memory Allocator: for_each_reserved_mem_range
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

int __init BiscuitOS_Running(void)
{
	phys_addr_t region_start, region_end;
	u64 idx;

	/* Add Fake Regions */
	memblock_add(MEMBLOCK_FAKE_MEMORY, MEMBLOCK_FAKE_SIZE);
	memblock_reserve(MEMBLOCK_FAKE_RESERVE, MEMBLOCK_FAKE_SIZE);

	/* Itereate over Reserved memory regions */
	printk("==== Iterator Reserved Region\n");
	for_each_reserved_mem_range(idx, &region_start, &region_end)
		printk("REGION %lld: %#llx - %#llx\n",
					idx, region_start, region_end);

	/* Remove Fake Regions */
	memblock_remove(MEMBLOCK_FAKE_MEMORY,  MEMBLOCK_FAKE_SIZE);
	memblock_remove(MEMBLOCK_FAKE_RESERVE, MEMBLOCK_FAKE_SIZE);

	return 0;
}
