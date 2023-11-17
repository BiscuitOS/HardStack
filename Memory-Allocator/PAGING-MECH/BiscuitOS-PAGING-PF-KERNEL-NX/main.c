// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault on Kernel: NX 
 *
 * (C) 2023.11.17 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>

/* Machine Code Running on X86 Kernel */
char MACH_CODE[] = { 0xc3, 0x00, 0x00 /* retq */ };

typedef void (*spur_func_t)(void);

static int __init BiscuitOS_init(void)
{
	spur_func_t func;
	void *mem;

	/* ALLOC VMALLOC MEMORY */
	mem = vzalloc(PAGE_SIZE);
	if (!mem)
		return -ENOMEM;

	/* INSTALL CODE */
	memcpy(mem, MACH_CODE, 3);
	func = (spur_func_t)mem;

	/* EXEC CODE, Trigger #KERNEL-PF */
	func();

	/* RECLAIM */
	vfree(mem);	

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
