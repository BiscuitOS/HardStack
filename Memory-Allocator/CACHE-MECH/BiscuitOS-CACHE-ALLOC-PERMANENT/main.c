// SPDX-License-Identifier: GPL-2.0-only
/*
 * CACHE Mode on Permanent Mapping Memory Allocator
 *
 * (C) 2023.04.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/fixmap.h>

#define BROILER_MMIO_BASE	0xD0000000
#define BROILER_FIXMAP_IDX	(__end_of_fixed_addresses - 1)

static int __init BiscuitOS_init(void)
{
	unsigned long addr;

	/* CACHE MODE: NOCACHE */
	__set_fixmap(BROILER_FIXMAP_IDX,
				BROILER_MMIO_BASE, FIXMAP_PAGE_NOCACHE);
	addr = fix_to_virt(BROILER_FIXMAP_IDX);

	printk("Broiler MMIO Read %#lx: %#x", addr, *(unsigned int *)addr);

	clear_fixmap(BROILER_FIXMAP_IDX);
	return 0;
}
device_initcall(BiscuitOS_init);
