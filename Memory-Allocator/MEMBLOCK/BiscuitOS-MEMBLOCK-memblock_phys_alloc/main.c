/*
 * MEMBLOCK Memory Allocator: memblock_phys_alloc
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
#define MEMBLOCK_FAKE_BASE	0x22000000
#define MEMBLOCK_FAKE_END	0x24000000
#define MEMBLOCK_FAKE_SIZE	0x10
#define MEMBLOCK_FAKE_NODE	1

int __init BiscuitOS_Running(void)
{
	phys_addr_t phys;
	void *mem;

	/* Alloc Physical Memory */
	phys = memblock_phys_alloc(MEMBLOCK_FAKE_SIZE, SMP_CACHE_BYTES);
	if (!phys) {
		printk("FATAL ERROR: No Free Memory on MEMBLOCK.\n");
		return -ENOMEM;
	}

	/* Cover physical to virtual */
	mem = phys_to_virt(phys);

	sprintf(mem, "Hello %s", "BiscuitOS");
	printk("==== %s ====\n", (char *)mem);
	printk("Address: Physical %#llx Virtual %#llx\n", phys, (u64)mem);

	/* Free Memory */
	memblock_phys_free(phys, MEMBLOCK_FAKE_SIZE);

	return 0;
}
