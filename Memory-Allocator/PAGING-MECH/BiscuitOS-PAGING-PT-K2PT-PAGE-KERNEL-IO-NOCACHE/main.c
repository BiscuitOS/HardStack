// SPDX-License-Identifier: GPL-2.0
/*
 * KERNEL With PageTable: PAGE_KERNEL_IO_NOCACHE
 *
 *    CMDLINE ADD "memmap=4K$0x10000000"
 *
 * (C) 2023.11.13 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/io.h>

#define RSVDMEM_BASE	0x10000000
#define RSVDMEM_SIZE	0x1000

static int __init BiscuitOS_init(void)
{
	void *base;
	
	base = ioremap_uc(RSVDMEM_BASE, RSVDMEM_SIZE);
	if (!base)
		return -ENOMEM;

	/* ACCESS */
	sprintf((char *)base, "Hello BiscuitOS");
	printk("PAGE_KERNEL_IO_UNCACHE: %s\n", (char *)base);

	/* RECLAIM */
	iounmap(base);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
