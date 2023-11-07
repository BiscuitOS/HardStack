// SPDX-License-Identifier: GPL-2.0
/*
 * Linear Mapping: Temporary mapping
 *
 * (C) 2023.10.28 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/mm.h>

static int __init BiscuitOS_init(void)
{
	struct page *page = alloc_page(GFP_KERNEL);
	void *addr;

	if (!page)
		return -ENOMEM;

	/* Temporary mapping */
	addr = kmap(page);

	/* Use memory */
	sprintf((char *)addr, "Hello BiscuitOS");
	printk("LM %#lx => %s\n", (unsigned long)addr, (char *)addr);

	/* unmapping */
	kunmap(page);

	/* Reclaim */
	__free_page(page);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS Paging Project");
