/*
 * VMALLOC with CACHE Mode
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <asm/io.h>

static int __init BiscuitOS_init(void)
{
	struct page *page;
	phys_addr_t base;
	pgprot_t prot;
	void *addr;

	/* Allocate memory */
	page = alloc_page(GFP_KERNEL);
	if (!page) {
		printk("System Error: No free memory.\n");
		return -ENOMEM;
	}
	base = PFN_PHYS(page_to_pfn(page));
	
	/* Change Memory Type for Direct-mapping area */
	arch_io_reserve_memtype_wc(base, PAGE_SIZE);
	prot = __pgprot_mask(__PAGE_KERNEL |
			cachemode2protval(_PAGE_CACHE_MODE_WC));

	/* Vmalloc mapping */
	addr = vmap(&page, 1, VM_MAP, prot);
	if (!addr) {
		printk("VMALLOC Mapping Failed.\n");
		__free_page(page);
		return -EINVAL;
	}

	/* Usage */
	sprintf((char *)addr, "Hello BiscuitOS");
	printk("%s\n  VA: %#lx\n  PA: %#llx\n",
				(char *)addr, (unsigned long)addr, base);

	/* Reclaim */
	vunmap(addr);
	arch_io_free_memtype_wc(base, PAGE_SIZE);
	__free_page(page);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("VMALLOC with CACHE Mode");
