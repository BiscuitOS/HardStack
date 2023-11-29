// SPDX-License-Identifier: GPL-2.0
/*
 * VTP: VMALLOC TO PFN
 *
 * (C) 2023.11.22 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>

static int __init BiscuitOS_init(void)
{
	unsigned long pfn;
	void *mem;

	/* ALLOC VMALLOC MEMORY */
	mem = vmalloc(PAGE_SIZE);
	if (!mem)
		return -ENOMEM;

	/* CONSULT PFN */
	pfn = vmalloc_to_pfn(mem);
	printk("VMALLOC %#lx MAPPING TO PFN %#lx\n",
				(unsigned long)mem, pfn);
	
	/* RECLAIM */
	vfree(mem);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
