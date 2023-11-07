// SPDX-License-Identifier: GPL-2.0
/*
 * CPA: Change Page Attribute
 *
 * (C) 2023.10.28 BuddyZhang1 <buddy.zhang@aliyun.com>
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gfp.h>
#include <asm/set_memory.h>

static int __init BiscuitOS_init(void)
{
	void *addr;

	/* Get kernel address */
	addr = (void *)__get_free_page(GFP_KERNEL);
	if (!addr) {
		printk("System no free memory!\n");
		return -ENOMEM;
	}

	/* Set virtual address as Write-Combining */
	set_memory_wc((unsigned long)addr, 1);

	sprintf((char *)addr, "Hello BiscuitOS");
	/* Info */
	printk("CPA KVaddr %#lx - %#lx\n%s\n", (unsigned long)addr,
			(unsigned long)addr + PAGE_SIZE, (char *)addr);

	/* Reset */
	set_memory_wb((unsigned long)addr, PAGE_SIZE);
	free_page((unsigned long)addr);

	return 0;
}
__initcall(BiscuitOS_init);
