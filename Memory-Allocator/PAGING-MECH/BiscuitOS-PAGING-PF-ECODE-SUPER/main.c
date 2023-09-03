// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault ERROR CODE: PF_SUPER
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	struct vm_struct *vm;
	void *addr;

	vm = get_vm_area(PAGE_SIZE, VM_ALLOC);
	if (!vm) {
		printk("VADDR Alloc Failed.\n");
		return -ENOMEM;
	}

	/* VMALLOC Virtual Address */
	addr = vm->addr;
	printk("PF-KERNEL: %#lx\n", (unsigned long)addr);

	/* Write Ops, Trigger #PF PF_SUPER */
	*(char *)addr = 'B';

	/* Remove */
	remove_vm_area(addr);

	return 0;
}
__initcall(BiscuitOS_init);
