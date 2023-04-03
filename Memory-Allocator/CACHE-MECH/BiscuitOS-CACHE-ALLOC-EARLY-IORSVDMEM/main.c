// SPDX-License-Identifier: GPL-2.0
/*
 * CACHE on EARLY IO/RSVDMEM Allocator
 *
 * Reserved memory: [64MiB, 64MiB + 4KiB)
 *   CMDLINE: memmap=4K$0x4000000
 *
 * (C) 2023.04.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/pgtable_types.h>
#include <asm-generic/early_ioremap.h>

#define RESERVED_MEMORY_BASE	0x4000000
#define RESERVED_MEMORY_SIZE	0x1000

int __init BiscuitOS_Running(void)
{
	void *mem;

	/* CACHE MODE: WP */
	mem = early_memremap_prot(RESERVED_MEMORY_BASE, 
			RESERVED_MEMORY_SIZE, __PAGE_KERNEL_NOENC_WP);
	if (!mem) {
		printk("EARLY-IOREMAP failed\n");
		return -ENOMEM;
	}

	sprintf((char *)mem, "Hello BiscuitOS");
	printk("EARLY IO/RSVDMEM CACHE: %s\n", (char *)mem);

	/* unmapping */
	early_memunmap(mem, RESERVED_MEMORY_SIZE);

	return 0;
}
