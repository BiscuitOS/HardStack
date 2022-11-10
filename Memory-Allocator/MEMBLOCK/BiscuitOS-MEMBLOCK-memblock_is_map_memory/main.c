/*
 * MEMBLOCK Memory Allocator: memblock_is_map_memory
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

#define MEMBLOCK_FAKE_SIZE	0x10

int __init BiscuitOS_Running(void)
{
	phys_addr_t phys;
	void *mem;

	/* Alloc Memory */
	mem = memblock_alloc(MEMBLOCK_FAKE_SIZE, SMP_CACHE_BYTES);
	if (!mem) {
		printk("FATAL ERROR: No Free Memory on MEMBLOCK.\n");
		return -ENOMEM;
	}

	phys = __pa(mem);
	printk("Virtual %#lx Physical %#llx\n", (unsigned long)mem, phys);

	/* Check mapping */
	if (memblock_is_map_memory(phys))
		printk("Physical %#llx has mapped memory!\n", phys);
	phys += SMP_CACHE_BYTES;
	if (memblock_is_map_memory(phys))
		printk("Physical %#llx has mapped memory!\n", phys);

	/* Free Memory */
	memblock_free(mem, MEMBLOCK_FAKE_SIZE);

	return 0;
}
