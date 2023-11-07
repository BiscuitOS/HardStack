// SPDX-License-Identifier: GPL-2.0
/*
 * LOOKUP-ADDRESS: VMALLOC
 *
 * (C) 2020.10.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>

static int __init BiscuitOS_init(void)
{
	void *vaddr;
	pte_t *pte;
	int level;
	
	/* ALLOC VMALLOC MEMORY */
	vaddr = vmalloc(PMD_SIZE);
	if (!vaddr)
		return -ENOMEM;

	/* LOOKUP PTE */
	pte = lookup_address((unsigned long)vaddr, &level);
	printk("PADDR: %#lx\nVADDR: %#lx\nPTE:   %#lx LEVEL %d\n",
			pte_pfn(*pte) << PAGE_SHIFT, (unsigned long)vaddr,
			pte_val(*pte), level);

	/* RECLAIM */
	vfree(vaddr);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
