/*
 * FIXMAP Memory Allocator: __fix_to_virt
 *
 * (C) 2022.10.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/fixmap.h>

#define BROILER_MMIO_BASE	0xD0000000
#define BROILER_FIXMAP_IDX	(__end_of_fixed_addresses - 1)

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	unsigned long addr;

	set_fixmap(BROILER_FIXMAP_IDX, BROILER_MMIO_BASE);
	addr = __fix_to_virt(BROILER_FIXMAP_IDX);

	printk("Broiler MMIO Read %#lx: %#x", addr, *(unsigned int *)addr);

	clear_fixmap(BROILER_FIXMAP_IDX);

	return 0;
}

device_initcall(BiscuitOS_init);
