// SPDX-License-Identifier: GPL-2.0
/*
 * PERMANENT MAPPING: MMIO
 *
 * (C) 2023.11.29 BuddyZhang1 <buddy.zhang@aliyun.com>
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

	/* MAPPING */
	addr = set_fixmap_offset(BROILER_FIXMAP_IDX, BROILER_MMIO_BASE);

	/* ACCESS */
	printk("Broiler MMIO Read %#lx: %#x", addr, *(unsigned int *)addr);

	/* UNMAPPING */
	clear_fixmap(BROILER_FIXMAP_IDX);

	return 0;
}
__initcall(BiscuitOS_init);
