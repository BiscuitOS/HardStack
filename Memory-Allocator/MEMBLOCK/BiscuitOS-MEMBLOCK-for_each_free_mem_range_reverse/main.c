/*
 * MEMBLOCK Memory Allocator: for_each_free_mem_range_reverse
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

	/* Itereate over memory regions on speical NUMA NODE */
	printk("==== Iterate Memory Region on NUMA %d\n", MEMBLOCK_FAKE_NODE);
	for_each_free_mem_range(idx, MEMBLOCK_FAKE_NODE,
			MEMBLOCK_NONE, &region_start, &region_end, &nid) {
		printk("RANGE%lld NID %d: %#llx - %#llx\n",
					idx, nid, region_start, region_end);
	} 

	/* Itereate over memory regions on speical NUMA NODE */
	printk("==== Iterate Memory Region on NUMA %d\n",
						MEMBLOCK_FAKE_NODE + 1);
	for_each_free_mem_range(idx, MEMBLOCK_FAKE_NODE + 1,
			MEMBLOCK_NONE, &region_start, &region_end, &nid) {
		printk("RANGE%lld NID %d: %#llx - %#llx\n",
					idx, nid, region_start, region_end);
	} 

	/* Reverse itereate over memory regions on speical NUMA NODE */
	printk("==== Reverse iterate Memory Region on NUMA %d\n",
						MEMBLOCK_FAKE_NODE);
	for_each_free_mem_range_reverse(idx, MEMBLOCK_FAKE_NODE,
			MEMBLOCK_NONE, &region_start, &region_end, &nid) {
		printk("RANGE%lld NID %d: %#llx - %#llx\n",
					idx, nid, region_start, region_end);
	} 

	
	/* Remove Fake Regions */
	memblock_remove(MEMBLOCK_FAKE_MEMORY,  MEMBLOCK_FAKE_SIZE);
	memblock_remove(MEMBLOCK_FAKE_RESERVE, MEMBLOCK_FAKE_SIZE);

	return 0;
}
