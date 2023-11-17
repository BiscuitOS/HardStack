// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault on Kernel: KPROBE-PF
 *
 * (C) 2023.11.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/vmalloc.h>

#define TRACK_ADDRESS	(0x6000000000)

static int handler_pre(struct kprobe *kp, struct pt_regs *regs)
{
	void *mem;

	if (regs->si != TRACK_ADDRESS)
		return 0;

	/* UNMAPPING KERNEL ADDRESS */
	mem = (void *)(VMALLOC_START - PAGE_SIZE);
	printk("UNMAP ADDRESS: %#lx\n", (unsigned long)mem);

	/* FORCE WRITE AND Trigger kprobe_page_fault */
	*(char *)mem = 'B';
	
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
