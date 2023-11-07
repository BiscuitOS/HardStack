// SPDX-License-Identifier: GPL-2.0
/*
 * Linear Mapping: MEMBLOCK Allocator
 *
 * (C) 2023.10.28 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memblock.h>

#define MEMBLOCK_FAKE_SIZE	0x10

int __init BiscuitOS_Running(void)
{
	void *mem;

	/* Alloc Memory */
	mem = memblock_alloc(MEMBLOCK_FAKE_SIZE, SMP_CACHE_BYTES);
	if (!mem) {
		printk("FATAL ERROR: No Free Memory on MEMBLOCK.\n");
		return -ENOMEM;
	}

	sprintf(mem, "Hello %s", "BiscuitOS");
	printk("LM %#lx => %s\n", (unsigned long)mem, (char *)mem);

	/* Free Memory */
	memblock_free(mem, MEMBLOCK_FAKE_SIZE);

	return 0;
}
