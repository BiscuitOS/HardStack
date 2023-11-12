// SPDX-License-Identifier: GPL-2.0
/*
 * LOOKUP-ADDRESS: PMD Entry
 *
 * (C) 2023.11.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	struct page *page;
	void *addr;
	pmd_t *pmd;

	/* ALLOC PHYSICAL ADDRESS */
	page = alloc_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;

	/* LINEAR-MAPPING TO VIRTUAL ADDRESS */
	addr = page_address(page);

	/* CONSULT PMD TABLE */
	pmd = lookup_pmd_address((unsigned long)addr);
	printk("PADDR: %#lx\nVADDR: %#lx\nPMD:   %#lx\n",
			pmd_pfn(*pmd), (unsigned long)addr,
			pmd_val(*pmd));

	/* RECLAIM */
	__free_page(page);

	return 0;
}
__initcall(BiscuitOS_init);
