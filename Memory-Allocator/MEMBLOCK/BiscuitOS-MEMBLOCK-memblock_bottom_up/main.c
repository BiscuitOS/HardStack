/*
 * MEMBLOCK Memory Allocator: memblock_bottom_up
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
	void *mem, *mem1;
	bool default_dirt = memblock_bottom_up();
	
	/* Alloc Memory from Bottom to Up */
	memblock_set_bottom_up(true);
	mem = memblock_alloc(MEMBLOCK_FAKE_SIZE, SMP_CACHE_BYTES);
	if (!mem) {
		printk("FATAL ERROR: No Free Memory on MEMBLOCK.\n");
		return -ENOMEM;
	}

	/* Alloc Memory from Up to Bottom */
	memblock_set_bottom_up(false);
	mem1 = memblock_alloc(MEMBLOCK_FAKE_SIZE, SMP_CACHE_BYTES);
	if (!mem1) {
		printk("FATAL ERROR: No Free Memory on mem1!\n");
		goto out1;
	}

	sprintf(mem, "Hello %s", "BiscuitOS");
	printk("==== %s ====\n", (char *)mem);
	printk("MEM %lx MEM1 %#lx\n", __pa(mem), __pa(mem1));

	/* Free Memory */
out1:
	memblock_free(mem1, MEMBLOCK_FAKE_SIZE);
	memblock_free(mem,  MEMBLOCK_FAKE_SIZE);
	memblock_set_bottom_up(default_dirt);

	return 0;
}
