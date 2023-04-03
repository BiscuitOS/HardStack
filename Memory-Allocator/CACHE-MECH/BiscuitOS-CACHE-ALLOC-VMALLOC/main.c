// SPDX-License-Identifier: GPL-2.0
/*
 * CACHE on VMALLOC Allocator
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>

static int __init BiscuitOS_init(void)
{
	struct page *page;
	void *addr;
	int r = 0;

	page = alloc_page(GFP_KERNEL);
	if (!page) {
		printk("System Error: Memory Emergency\n");
		return -ENOMEM;
	}

	/* VMALLOC Mapping */
	addr = vmap(&page, 1, VM_MAP, PAGE_KERNEL_NOCACHE);
	if (!addr) {
		printk("VMALLOC Error: Mapping failed.\n");
		r = -EBUSY;
		goto err_vmap;
	}

	/* Use */
	sprintf((char *)addr, "Hello BiscuitOS");
	printk("%s\n", (char *)addr);

	/* Reclaim */
	vfree(addr);

err_vmap:
	__free_page(page);
	return r;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("CACHE on VMALLOC Allocator");
