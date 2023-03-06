/*
 * WriteThrough(WT) Memory on Kernel
 *
 * (C) 2023.02.10 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gfp.h>
#include <asm/set_memory.h>

static int __init BiscuitOS_init(void)
{
	void *addr;

	addr = (void *)__get_free_page(GFP_KERNEL);
	if (!addr) {
		printk("System no free memory!\n");
		return -ENOMEM;
	}

	/* Set virtual address as WriteThrough(WT) */
	_set_memory_wt((unsigned long)addr, PAGE_SIZE);

	sprintf((char *)addr, "Hello BiscuitOS");
	/* Info */
	printk("Virtual %#lx - %#lx\n%s\n", (unsigned long)addr,
			(unsigned long)addr + PAGE_SIZE, (char *)addr);

	/* Reclaim */
	set_memory_wb((unsigned long)addr, PAGE_SIZE);
	free_page((unsigned long)addr);
	return 0;
}
device_initcall(BiscuitOS_init);
