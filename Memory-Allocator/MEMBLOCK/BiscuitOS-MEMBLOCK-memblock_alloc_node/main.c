/*
 * MEMBLOCK Memory Allocator: memblock_alloc_node
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
#define MEMBLOCK_FAKE_SIZE	0x10
#define MEMBLOCK_FAKE_NODE	1

int __init BiscuitOS_Running(void)
{
	void *mem;

	/* Alloc Memory */
	mem = memblock_alloc_node(MEMBLOCK_FAKE_SIZE,
				SMP_CACHE_BYTES, MEMBLOCK_FAKE_NODE);
	if (!mem) {
		printk("FATAL ERROR: No Free Memory on MEMBLOCK.\n");
		return -ENOMEM;
	}

	sprintf(mem, "Hello %s", "BiscuitOS");
	printk("==== %s ====\n", (char *)mem);
	printk("Physical Address %#lx\n", __pa(mem));

	/* Free Memory */
	memblock_free(mem, MEMBLOCK_FAKE_SIZE);

	return 0;
}
