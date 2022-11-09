/*
 * MEMBLOCK Memory Allocator: memblock_is_region_memory
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

#define MEMBLOCK_FAKE_BASE	0x800000000
#define MEMBLOCK_FAKE_SIZE	0x100000

int __init BiscuitOS_Running(void)
{
	/* Add new range */
	memblock_add(MEMBLOCK_FAKE_BASE, MEMBLOCK_FAKE_SIZE);

	if (memblock_is_region_memory(MEMBLOCK_FAKE_BASE, 0x20))
		printk("Range %#lx is Memory.\n", MEMBLOCK_FAKE_BASE);

	/* Only Test: Remove range */
	memblock_remove(MEMBLOCK_FAKE_BASE, MEMBLOCK_FAKE_SIZE);

	return 0;
}
