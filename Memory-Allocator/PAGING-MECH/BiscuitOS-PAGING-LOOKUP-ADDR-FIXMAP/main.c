// SPDX-License-Identifier: GPL-2.0
/*
 * LOOKUP-ADDRESS: FIXMAP or Permanent Mapping
 *
 * (C) 2023.11.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/fixmap.h>

#define BROILER_MMIO_BASE	0xD0000000UL
#define BROILER_FIXMAP_IDX	(__end_of_fixed_addresses - 1)

static int __init BiscuitOS_init(void)
{
	unsigned long vaddr;
	pte_t *pte;
	int level;

	/* ALLOC FIXMAP MEMORY */
	set_fixmap(BROILER_FIXMAP_IDX, BROILER_MMIO_BASE);
	vaddr = fix_to_virt(BROILER_FIXMAP_IDX);

        /* LOOKUP PTE */
	pte = lookup_address((unsigned long)vaddr, &level);
	printk("PADDR: %#lx\nVADDR: %#lx\nPTE:   %#lx LEVEL %d\n",
			BROILER_MMIO_BASE, vaddr,
			pte_val(*pte), level);

	/* RECLAIM */
	clear_fixmap(BROILER_FIXMAP_IDX);

	return 0;
}
__initcall(BiscuitOS_init);
