// SPDX-License-Identifier: GPL-2.0
/*
 * MEMORY FLUID: KERNEL SPACE
 *
 * (C) 2023.11.30 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>

static int __init BiscuitOS_init(void)
{
	struct page *page;
	void *mem;

	/* ALLOC VMALLOC MEMORY */
	mem = vmalloc(PAGE_SIZE);
	if (!mem)
		return -ENOMEM;

	/* CONSULT PAGE */
	page = vmalloc_to_page(mem);
	printk("VMALLOC %#lx MAPPING TO %#lx\n",
		(unsigned long)mem, page_to_pfn(page) << PAGE_SHIFT);
	
	/* RECLAIM */
	vfree(mem);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS MEMORY FLUID");
