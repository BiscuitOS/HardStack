/*
 * MEMBLOCK Memory Allocator: physmem
 *
 * - Must Reserved Memory on CMDLINE: 
 *   'memmap=1M$0x10000000 memmap=1M$0x10200000 memmap=1M$0x10400000'
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
#include <asm/e820/types.h>

int __init BiscuitOS_Running(void)
{
	phys_addr_t start, end;
	u64 idx;

	/* Iterate all memory: Available + Reserved */
	printk("===== System Memory =====\n");
	for_each_physmem_range(idx, NULL, &start, &end)
		printk("Region: %#llx - %#llx\n", start, end);

	/* Iterate Reserved Memory */
	printk("===== Reserved Memory =====\n");
	for_each_physmem_range(idx, &memblock.memory, &start, &end)
		printk("Region: %#llx - %#llx\n", start, end);

	return 0;
}
