// SPDX-License-Identifier: GPL-2.0
/*
 * PAGING TOOLS: Kprobe
 *
 * (C) 2023.11.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

#define TRACK_ADDRESS	(0x6000000000)

static int handler_pre(struct kprobe *kp, struct pt_regs *regs)
{
	struct vm_area_struct *vma;

	if (regs->si != TRACK_ADDRESS)
		return 0;

	vma = (struct vm_area_struct *)regs->di;

	printk("TRACK: %#lx - %#lx\n", vma->vm_start, vma->vm_end);
	printk("FAULT Address: %#lx\n", (unsigned long)regs->si);
	printk("FAULT Reason:  %#lx\n", (unsigned long)regs->dx);

	return 0;
}

static struct kprobe kp = {
	.symbol_name	= "handle_mm_fault",
	.pre_handler	= handler_pre,
};

static int __init BiscuitOS_init(void)
{
	int r;

	r = register_kprobe(&kp);
	if (r < 0) {
		printk("REGISTER KPROBE FAILED.\n");
		return r;
	}

	return 0;
}

static void __exit BiscuitOS_exit(void)
{
	unregister_kprobe(&kp);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
