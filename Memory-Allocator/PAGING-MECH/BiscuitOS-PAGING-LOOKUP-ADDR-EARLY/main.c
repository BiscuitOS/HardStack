// SPDX-License-Identifier: GPL-2.0
/*
 * LOOKUP-ADDRESS: EARLY MEMREMAP
 *
 *   CMDLINE ADD "memmap=4K$0x10000000"
 *
 * (C) 2023.11.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm-generic/early_ioremap.h>
#include <linux/mm.h>

#define RESERVED_MEMORY_BASE	0x10000000UL
#define RESERVED_MEMORY_SIZE	0x1000UL

int __init BiscuitOS_Running(void)
{
	void *vaddr;
	pte_t *pte;
	int level;

	/* ALLOC EARLY MEMREMAP MEMORY */
	vaddr = early_memremap(RESERVED_MEMORY_BASE, RESERVED_MEMORY_SIZE);
	if (!vaddr)
		return -ENOMEM;

	/* LOOKUP PTE */
	pte = lookup_address((unsigned long)vaddr, &level);
	printk("PADDR: %#lx\nVADDR: %#lx\nPTE:   %#lx LEVEL %d\n",
			RESERVED_MEMORY_BASE, (unsigned long)vaddr,
			pte_val(*pte), level);

	/* RECLAIM */
	early_memunmap(vaddr, RESERVED_MEMORY_SIZE);

	return 0;
}
