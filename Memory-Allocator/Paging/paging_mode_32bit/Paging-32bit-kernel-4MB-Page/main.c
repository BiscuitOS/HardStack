/*
 * 4M Physical Page With 32-Bit Paging
 *
 * (C) 2021.01.10 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gfp.h>
/* page table */
#include <asm/pgalloc.h>

/* BiscuitOS Emulate 4M Virtual Area */
#define BISCUITOS_4M_BASE		((PAGE_OFFSET - 0x1800000) & PMD_MASK)
/* 4M Page */
#define _PAGE_4M			(_PAGE_PRESENT | _PAGE_RW | \
					 _PAGE_ACCESSED | _PAGE_DIRTY | \
					 _PAGE_ENC | __PAGE_KERNEL | \
					 _PAGE_PSE)

/* Build PDE with 4M Page */
static void *BiscuitOS_32bit_paging_4M(struct page *page)
{
	pgd_t *pgd_base = swapper_pg_dir;
	unsigned long pfn = page_to_pfn(page);
	unsigned long vaddr;
	pgd_t *pgd;
	pmd_t *pde;

	vaddr = BISCUITOS_4M_BASE;
	pgd = pgd_base + pgd_index(vaddr);

	pde = (pmd_t *)pgd;
	set_pmd(pde, pfn_pmd(pfn, __pgprot(_PAGE_4M)));
	flush_tlb_kernel_range(vaddr, vaddr + (1 << 22));
	return (void *)vaddr;
}

/* Release PDE with 4M Page */
static void BiscuitOS_32bit_paging_4M_unmap(unsigned long vaddr)
{
	pgd_t *pgd_base = swapper_pg_dir;
	pgd_t *pgd = pgd_base + pgd_index(vaddr);
	pmd_t *pde;

	pde = (pmd_t *)pgd;
	pmd_clear(pde);
	flush_tlb_kernel_range(vaddr, vaddr + (1 << 22));
}

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	struct page *page;
	unsigned long *addr;
	unsigned long *addr2;

	/* Alloc 4M Physical Page */
	page = alloc_pages(GFP_KERNEL | __GFP_HIGHMEM, 10);
	if (!page)
		return -ENOMEM;

	/* Mapping for 4M Page  */
	addr  = (unsigned long *)BiscuitOS_32bit_paging_4M(page);
	addr2 = (unsigned long *)((unsigned long)addr + (1 << 20));

	*addr  = 88520;
	*addr2 = 52088;
	printk("\n\n\n\n**************BiscuitOS*****************\n");
	printk("=> %#lx: %ld\n", (unsigned long)addr, *addr);
	printk("=> %#lx: %ld\n", (unsigned long)addr2, *addr2);
	printk("******************************************\n\n\n\n");

	/* Unmapping for 4M Page */
	BiscuitOS_32bit_paging_4M_unmap((unsigned long)addr);

	/* Trigger kernel panic if access addr */
	//*addr = 88520;

	/* Free 4M Physical Page */
	__free_pages(page, 10);
	return 0;
}
device_initcall(BiscuitOS_init);
