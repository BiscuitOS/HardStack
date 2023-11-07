// SPDX-License-Identifier: GPL-2.0
/*
 * LOOKUP-ADDRESS
 *
 * (C) 2023.11.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>

static int __init BiscuitOS_init(void)
{
	struct page *page;
	void *vaddr;
	pte_t *pte;
	int level;
	
	/* ALLOC PHYSICAL MEMORY */
	page = alloc_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;

	/* LINEAR MAPPING: TO VIRTUAL */
	vaddr = page_address(page);

	/* LOOKUP PTE */
	pte = lookup_address((unsigned long)vaddr, &level);
	printk("PADDR: %#lx\nVADDR: %#lx\nPTE:   %#lx LEVEL %d\n",
			page_to_pfn(page), (unsigned long)vaddr,
			pte_val(*pte), level);

	/* RECLAIM */
	__free_page(page);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
