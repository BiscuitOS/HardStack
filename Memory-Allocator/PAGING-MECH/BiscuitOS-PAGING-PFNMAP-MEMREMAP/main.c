// SPDX-License-Identifier: GPL-2.0
/*
 * PFNMAP: MEMREMAP
 *
 *   - CMDLINE ADD "memmap=4K$0x10000000"
 *
 * (C) 2023.11.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>

#define RSVDMEM_BASE	0x10000000
#define RSVDMEM_SIZE	0x1000
static void *addr;

static int __init BiscuitOS_init(void)
{
	/* PFNMAPPING */
	addr = memremap(RSVDMEM_BASE, RSVDMEM_SIZE, MEMREMAP_WB);
	if (!addr) {
		printk("ERROR: memremap failed.\n");
		return -ENOMEM;
	}

	/* ACCESS RSVDMEM */
	*(char *)addr = 'B';
	printk("MEMREMAP: %#lx => %c\n", (unsigned long)addr, *(char *)addr);

	return 0;
}

static void __exit BiscuitOS_exit(void)
{
	/* RECLAIM */
	memunmap(addr);	
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
