/*
 * FIXMAP Allocator With 32-bit Paging
 *
 * (C) 2021.01.10 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/highmem.h>
/* page table */
#include <asm/pgalloc.h>
#include <asm/fixmap.h>

/* counter */
static int BiscuitOS_fixmaps_set;

static void __BiscuitOS_native_set_fixmap(enum fixed_addresses idx, pte_t pte)
{
	unsigned long address = __fix_to_virt(idx);

	if (idx >= __end_of_fixed_addresses) {
		BUG();
		return;
	}
	set_pte_vaddr(address, pte);
	BiscuitOS_fixmaps_set++;
}

static void BiscuitOS_native_set_fixmap(enum fixed_addresses idx,
					phys_addr_t phys, pgprot_t flags)
{
	/* Sanitize 'prot' against any unsupported bits */
	pgprot_val(flags) &= __default_kernel_pte_mask;

	__BiscuitOS_native_set_fixmap(idx, pfn_pte(phys >> PAGE_SHIFT, flags));
}

static inline void BiscuitOS_set_fixmap(enum fixed_addresses idx,
					phys_addr_t phys, pgprot_t flags)
{
	BiscuitOS_native_set_fixmap(idx, phys, flags);
}

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	struct page *page;
	phys_addr_t phys;
	unsigned long *addr = (unsigned long *)fix_to_virt(FIX_HOLE);

	/* Emulate Physical Address for IO Device */
	page = alloc_page(GFP_KERNEL | __GFP_HIGHMEM);
	if (!page)
		return -ENOMEM;
	phys = page_to_phys(page);

	/* FIXED Map */
	BiscuitOS_set_fixmap(FIX_HOLE, phys, FIXMAP_PAGE_NOCACHE);

	*addr = 88520;

	printk("\n\n\n\n*************BiscuitOS*************\n");
	printk("=> %#lx: %ld\n", (unsigned long)addr, *addr);
	printk("**********************************\n\n\n\n");

	/* FIXED Unmap */
	BiscuitOS_set_fixmap(FIX_HOLE, 0, __pgprot(0));

	/* Trigger kernel panic if access addr */
	//*addr = 88520;

	__free_page(page);
	return 0;
}
device_initcall(BiscuitOS_init);
