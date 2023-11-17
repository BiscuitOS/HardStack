// SPDX-License-Identifier: GPL-2.0
/*
 * KERNEL With PageTable
 *
 * (C) 2023.11.13 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>

static int __init BiscuitOS_init(void)
{
	void *mem;

	/* ALLOC PAGE_KERNEL MEMORY */
	mem = vmalloc(PAGE_SIZE);
	if (!mem)
		return -ENOMEM;

	/* ACCESS MEMORY */
	sprintf((char *)mem, "Hello BiscuitOS");
	printk("PAGE-KERNEL: %s\n", (char *)mem);

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
