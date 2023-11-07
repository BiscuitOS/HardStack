// SPDX-License-Identifier: GPL-2.0
/*
 * VMALLOC Mapping
 *
 * (C) 2023.10.31 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>

static int __init BiscuitOS_init(void)
{
	void *addr;

	/* Alloc from VMALLOC */
	addr = vmalloc(PMD_SIZE);

	/* Use memory */
	sprintf((char *)addr, "Hello BiscuitOS");
	printk("VMALLOC %#lx => %s\n", (unsigned long)addr, (char *)addr);

	/* Reclaim */
	vfree(addr);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS Paging Project");
