/*
 * MEMBLOCK Memory Allocator: memblock_get_current_limit
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

#define MEMBLOCK_FAKE_LIMIT	0x2000000
#define MEMBLOCK_FAKE_SIZE	0x10

int __init BiscuitOS_Running(void)
{
	phys_addr_t limit;
	void *mem;

	/* Default limit */
	limit = memblock_get_current_limit();
	/* Fake limit */
	memblock_set_current_limit(MEMBLOCK_FAKE_LIMIT);

	/* Alloc Memory */
	mem = memblock_alloc(MEMBLOCK_FAKE_SIZE, SMP_CACHE_BYTES);
	if (!mem) {
		printk("FATAL ERROR: No Free Memory on MEMBLOCK.\n");
		return -ENOMEM;
	}

	sprintf(mem, "Hello %s", "BiscuitOS");
	printk("==== %s ====\n", (char *)mem);

	/* Free Memory */
	memblock_free(mem, MEMBLOCK_FAKE_SIZE);

	/* Recove limit */
	memblock_set_current_limit(limit);

	return 0;
}
