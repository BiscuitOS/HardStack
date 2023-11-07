// SPDX-License-Identifier: GPL-2.0
/*
 * LOOKUP-ADDRESS: MEMBLOCK
 *
 * (C) 2023.11.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memblock.h>

#define MEMBLOCK_FAKE_SIZE	0x10

int __init BiscuitOS_Running(void)
{
	void *vaddr;
	pte_t *pte;
	int level;

	/* ALLOC MEMBLOCK MEMORY */
	vaddr = memblock_alloc(MEMBLOCK_FAKE_SIZE, SMP_CACHE_BYTES);
	if (!vaddr)
		return -ENOMEM;

	/* LOOKUP PTE */
	pte = lookup_address((unsigned long)vaddr, &level);
	printk("PADDR: %#lx\nVADDR: %#lx\nPTE:   %#lx LEVEL %d\n",
			pte_pfn(*pte) << PAGE_SHIFT, (unsigned long)vaddr,
			pte_val(*pte), level);

	/* RECLAIM */
	memblock_free(vaddr, MEMBLOCK_FAKE_SIZE);

	return 0;
}
