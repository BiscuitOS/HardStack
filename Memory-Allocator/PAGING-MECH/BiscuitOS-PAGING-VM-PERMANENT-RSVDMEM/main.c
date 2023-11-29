// SPDX-License-Identifier: GPL-2.0
/*
 * PERMANENT MAPPING: RSVDMEM
 *
 *  CMDLINE ADD "memmap=4K$0x10000000"
 *
 * (C) 2023.11.29 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/fixmap.h>

#define RSVDMEM_BASE		0x10000000
#define RSVDMEM_FIXMAP_IDX	(__end_of_fixed_addresses - 1)

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	unsigned long addr;

	/* MAPPING */
	addr = set_fixmap_offset(RSVDMEM_FIXMAP_IDX, RSVDMEM_BASE);

	/* ACCESS */
	printk("RSVDMEM Read %#lx: %#x", addr, *(unsigned int *)addr);

	/* UNMAPPING */
	clear_fixmap(RSVDMEM_FIXMAP_IDX);

	return 0;
}
__initcall(BiscuitOS_init);
