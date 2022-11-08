/*
 * MEMBLOCK Memory Allocator: memblock_overlaps_region
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

#define MEMBLOCK_FAKE_BASE	0xC000000
#define MEMBLOCK_FAKE_SIZE	0x100000

int __init BiscuitOS_Running(void)
{
	printk("RANGE %#x - %#x ", MEMBLOCK_FAKE_BASE, MEMBLOCK_FAKE_SIZE);

	/* Overlap Check */
	if (memblock_overlaps_region(&memblock.memory,
				MEMBLOCK_FAKE_BASE, MEMBLOCK_FAKE_SIZE))
		printk("====== OVERLAPS! ======\n");
	else
		printk("====== Avaiable! ======\n");

	return 0;
}
