/*
 * Trigger page fault for Kernel Space virtual address
 *
 * (C) 2023.02.12 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	struct vm_struct *area;
	void *vaddr;

	area = get_vm_area(PAGE_SIZE, VM_IOREMAP);
	if (!area) {
		printk("System no free memory!\n");
		return -ENOMEM;
	}
	/* Kernel Space virtual address */
	vaddr = area->addr;
	printk("Kernel Virtual Address: %#lx\n", (unsigned long)vaddr);

	/* Trigger page-fault for Kernel Space virtual address */
	*(char *)vaddr = 'B'; /* BUG! */

	/* Reclaim */
	free_vm_area(area);

	return 0;
}
device_initcall(BiscuitOS_init);
