// SPDX-License-Identifier: GPL-2.0
/*
 * KERNEL with PageTable: PAGE_KERNEL_NOENC
 *
 *   CMDLINE ADD "memmap=4K$0x10000000"
 *
 * (C) 2023.11.13 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm-generic/early_ioremap.h>
#include <linux/mm.h>

#define RESERVED_MEMORY_BASE	0x10000000UL
#define RESERVED_MEMORY_SIZE	0x1000UL

int __init BiscuitOS_Running(void)
{
	void *mem;

	/* ALLOC EARLY MEMREMAP MEMORY */
	mem = early_memremap_prot(RESERVED_MEMORY_BASE, 
			RESERVED_MEMORY_SIZE, pgprot_val(PAGE_KERNEL_NOENC));
	if (!mem)
		return -ENOMEM;

	/* ACCESS */
	sprintf((char *)mem, "Hello BiscuitOS");
	printk("PAGE_KERNEL_NOENC: %s\n", (char *)mem);

	/* RECLAIM */
	early_memunmap(mem, RESERVED_MEMORY_SIZE);

	return 0;
}
