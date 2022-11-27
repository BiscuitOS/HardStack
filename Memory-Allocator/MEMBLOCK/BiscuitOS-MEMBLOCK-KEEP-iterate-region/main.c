/*
 * MEMBLOCK Keeping Work: Iterate Region
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

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	phys_addr_t start, end;
	u64 idx;

	printk("Iterate Available Memory\n");
	for_each_mem_range(idx, &start, &end)
		printk("Range %lld: %#llx - %#llx\n", idx, start, end);

	return 0;
}

device_initcall(BiscuitOS_init);
