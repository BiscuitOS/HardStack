// SPDX-License-Identifier: GPL-2.0
/*
 * KERNEL With PageTable: PAGE_KERNEL_NOCACHE
 *
 * (C) 2023.11.13 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>

#define PAGE_AGP	PAGE_KERNEL_NOCACHE

static int __init BiscuitOS_init(void)
{
	struct page *page;
	void *mem;

	/* ALLOC PHYSICAL MEMORY */
	page = alloc_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;

	/* ALLOC PAGE_KERNEL_NOCACHE MEMORY */
	mem = vmap(&page, 1, VM_IOREMAP, PAGE_AGP);
	if (!mem) {
		__free_page(page);
		return -ENOMEM;
	}

	/* ACCESS MEMORY */
	sprintf((char *)mem, "Hello BiscuitOS");
	printk("PAGE-KERNEL-NOCACHE: %s\n", (char *)mem);

	/* RECLAIM */
	vunmap(mem);
	__free_page(page);
	
	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
