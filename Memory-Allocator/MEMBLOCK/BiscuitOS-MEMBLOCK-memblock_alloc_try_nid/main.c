/*
 * MEMBLOCK Memory Allocator: memblock_alloc_try_nid
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
/* NUMA RANGE
 *  NODE0: 0x00000000 - 0x20000000
 *  NODE1: 0x20000000 - 0x30000000
 *  NODE2: 0x30000000 - 0x40000000
 */
#define MEMBLOCK_FAKE_BASE	0x20000000
#define MEMBLOCK_FAKE_END	0x30000000
#define MEMBLOCK_NODE		1

int __init BiscuitOS_Running(void)
{
	void *mem;

	bs_debug_enable();
	mem = memblock_alloc_try_nid(MEMBLOCK_FAKE_SIZE, SMP_CACHE_BYTES,
			MEMBLOCK_FAKE_BASE, MEMBLOCK_FAKE_END, MEMBLOCK_NODE);
	bs_debug_disable();
	if (!mem) {
		printk("%#x - %#x no free memory!\n",
				MEMBLOCK_FAKE_BASE, MEMBLOCK_FAKE_END);
		return -ENOMEM;
	}

	sprintf(mem, "==== Hello %s ====\n", "BiscuitOS");
	printk("[%#lx] %s", __pa(mem), (char *)mem);

	memblock_free(mem, MEMBLOCK_FAKE_SIZE);

	return 0;
}
