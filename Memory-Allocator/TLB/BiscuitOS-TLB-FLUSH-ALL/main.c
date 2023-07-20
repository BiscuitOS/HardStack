// SPDX-License-Identifier: GPL-2.0
/*
 * FLUSH ALL TLB
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/tlbflush.h>

static int __init BiscuitOS_init(void)
{
	__flush_tlb_all();
	printk("Flush ALL TLB on BiscuitOS\n");

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("FLUSH TLB");
