/*
 * MTRRs: View Cache information from MTRR and PAT
 *   
 * - Must Reserved Memory 1KiB on 0x10000000, Add to Cmdline "memmap=4K$0x10000000"
 * - Cat /sys/kernel/debug/x86/pat_memtype_list
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <asm/io.h>

#define FAKE_MEM_BASE		0x10000000
#define FAKE_MEM_SIZE		0x1000

static int __init BiscuitOS_init(void)
{
	char *vaddr;

	/* Check memory isn't system ram */
	if (page_is_ram(FAKE_MEM_BASE >> PAGE_SHIFT)) {
		printk("Must Reserved Memory %#x on CMDLINE.\n"
			"Such as %#x$%#x", FAKE_MEM_BASE, FAKE_MEM_SIZE,
					   FAKE_MEM_BASE);
		return -EINVAL;
	}

	/* IOREMAP */
	vaddr = (char *)ioremap_uc(FAKE_MEM_BASE, FAKE_MEM_SIZE);
	if (!vaddr) {
		printk("IOREMAP Failed.\n");
		return -EINVAL;
	}

	sprintf(vaddr, "Hello %s", "BiscuitOS");
	printk("=== %s ===\n", vaddr);

	/* 
	 * No free vaddress, cat cache information from:
	 * /sys/kernel/debug/x86/pat_memtype_list
	 */
	//iounmap(vaddr);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("X86 MTRRs on BiscuitOS");
