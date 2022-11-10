/*
 * MEMBLOCK Memory Allocator: memblock_phys_mem_size
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
	phys_addr_t val;

	/* Total Physical Memory */
	val = memblock_phys_mem_size();
	printk("MEMBLOCK contain %#llx physical memory!\n", val);

	/* Reserved Memory */
	val = memblock_reserved_size();
	printk("MEMBLOCK contain %#llx reserved memory!\n", val);

	/* MEMBLOCK Range */
	printk("MEMBLOCK Range %#llx - %#llx\n",
		memblock_start_of_DRAM(), memblock_end_of_DRAM());

	return 0;
}
