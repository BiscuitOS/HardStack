// SPDX-License-Identifier: GPL-2.0
/*
 * KERNEL With PageTable: PAGE_KERNEL_EXEC
 *
 * (C) 2023.11.13 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/moduleloader.h>

static int __init BiscuitOS_init(void)
{
	void *mem;

	/* ALLOC PAGE_KERNEL_EXEC MEMORY */
	mem = module_alloc(PAGE_SIZE);
	if (!mem)
		return -ENOMEM;

	/* ACCESS */
	sprintf((char *)mem, "Hello BiscuitOS");
	printk("PAGE_KERNEL_EXEC: %s\n", (char *)mem);

	/* RECLAIM */
	vfree(mem);

	return 0;
}
__initcall(BiscuitOS_init);
