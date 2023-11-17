// SPDX-License-Identifier: GPL-2.0
/*
 * KERNEL With PageTable: PAGE_KERNEL_ROX
 *
 * (C) 2023.11.13 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/kprobes.h>

static int __init BiscuitOS_init(void)
{
	void *page;

	/* ALLOC PAGE_KERNEL_ROX MEMORY */
	page = alloc_insn_page();
	if (!page)
		return -ENOMEM;

	/* ACCESS */
	sprintf((char *)page, "Hello BiscuitOS");
	printk("PAGE_KERNEL_EXEC: %s\n", (char *)page);

	/* RECLAIM */
	vfree(page);

	return 0;
}
__initcall(BiscuitOS_init);
