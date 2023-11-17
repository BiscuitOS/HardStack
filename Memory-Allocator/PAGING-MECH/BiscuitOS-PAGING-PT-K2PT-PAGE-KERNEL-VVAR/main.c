// SPDX-License-Identifier: GPL-2.0-only
/*
 * KERNEL With PageTable: PAGE_KERNEL_VVAR
 *
 * (C) 2023.11.13 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/fixmap.h>

#define RSVDMEM_BASE	0x10000000
#define FIXMAP_IDX	(__end_of_fixed_addresses - 1)

static int __init BiscuitOS_init(void)
{
	unsigned long addr;

	/* MAPPING GLOBAL MEMORY */
	__set_fixmap(FIXMAP_IDX, RSVDMEM_BASE, PAGE_KERNEL_VVAR);
	addr = fix_to_virt(FIXMAP_IDX);

	/* ACCESS */
	sprintf((char *)addr, "Hello BiscuitOS");
	printk("PAGE_KERNEL_VVAR: %s\n", (char *)addr);

	/* RECLAIM */
	clear_fixmap(FIXMAP_IDX);

	return 0;
}
__initcall(BiscuitOS_init);
