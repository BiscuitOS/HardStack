/*
 * CRASH: ptov
 *
 * (C) 2021.06.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	struct vm_area_struct __percpu *BiscuitOS_percpu;
	struct page *page;
	phys_addr_t phys;
	void *vaddr;

	/* Page */
	page = alloc_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;

	vaddr = page_address(page);
	phys  = page_to_phys(page);
	printk("Page PHYS: %#lx VADDR: %#lx\n", (unsigned long)phys, vaddr);

	/* PERCPU */
	BiscuitOS_percpu = alloc_percpu(struct vm_area_struct);
	if (!BiscuitOS_percpu)
		return -ENOMEM;

	vaddr = BiscuitOS_percpu;
	phys = virt_to_phys(BiscuitOS_percpu);
	printk("Page PHYS: %#lx VADDR: %#lx\n", (unsigned long)phys, vaddr);

	/* panic for CRASH */
	panic("CRASH ptov");

	free_percpu(BiscuitOS_percpu);
	__free_page(page);

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void) {}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("CRASH ptov on BiscuitOS");
