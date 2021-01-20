/*
 * MEMBLOCK: memblock_set_region_node
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memblock.h>

extern struct memblock memblock;

int __init BiscuitOS_Running(void)
{
	phys_addr_t pesudo_start, pesudo_size;
	struct memblock_region *rgn;

	/* Pesudo Range */
	pesudo_start = 0x800000000000;
	pesudo_size  = 0x100000;

	/* MEMBLOCK Add Region */
	memblock_add(pesudo_start, pesudo_size);

	for_each_memblock(memory, rgn) {
		if (rgn->base == pesudo_start && rgn->size == pesudo_size) {
			memblock_set_region_node(rgn, numa_node_id());
			break;
		}
	}

	/* NUMA NODE for region */
	printk("The NUMA NODE for pesudo region: %d\n", 
				memblock_get_region_node(rgn));

	/* MEMBLOCK Remove Region */
	memblock_remove(pesudo_start, pesudo_size);

	printk("Hello BiscuitOS anywhere on kernel.\n");

	return 0;
}
