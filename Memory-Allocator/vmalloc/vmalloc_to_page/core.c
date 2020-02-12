/*
 * VMALLOC
 *
 * (C) 2020.02.12 BuddyZhang1 <buddy.zhang@aliyun.com>
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

/* Module initialize entry */
static int __init Demo_init(void)
{
	struct page *page;
	unsigned long *p;

	/* alloc */
	p = vmalloc(sizeof(unsigned long));
	if (!p) {
		printk("Vmalloc no free memory.\n");
		return -ENOMEM;
	}

	*p = 0x909192;
	printk("Value %#lx ADDR %#lx\n", *p, (unsigned long)p);

	/* vaddr to page */
	page = vmalloc_to_page(p);
	printk("Page-2-PFN %#lx\n", page_to_pfn(page));

	/* free */
	vfree(p);

	return 0;
}

/* Module exit entry */
static void __exit Demo_exit(void)
{
}

module_init(Demo_init);
module_exit(Demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("VMALLOC Memory Allocator");
