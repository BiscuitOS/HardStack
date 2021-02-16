/*
 * Physical Frame Direct Mapping With 32-Bit Paging
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
#include <linux/highmem.h>
/* page table */
#include <asm/pgalloc.h>
/* max_pfn */
#include <linux/memblock.h>

/* Physical Frame */
#define BISCUITOS_BASE_PFN		(max_pfn+1)
#define BISCUITOS_SIZE			PAGE_SIZE
/* Virtual Address */
#define BISCUITOS_VADDR			(VMALLOC_START + 0x700000)

/* Build Page Table */
static void *BiscuitOS_build_page_table(unsigned long vaddr, 
					unsigned long pfn, unsigned long size)
{
	pgd_t *pgd;
	pmd_t *pde;
	pte_t *pte;

	pgd = pgd_offset_k(vaddr);
	/* 32-bit Paging only contains PDE and PTE */
	pde = (pmd_t *)pgd;
	if (!pmd_val(*pde) & _PAGE_PRESENT) {
		pte_t *page_table;
		struct page *page;

		/* Page Table */
		page = alloc_page(GFP_KERNEL);
		if (!page)
			return NULL;

		page_table = page_address(page);
		/* Setup PDE */
		paravirt_alloc_pte(&init_mm, __pa(page_table) >> PAGE_SHIFT);
		set_pmd(pde, __pmd(__pa(page_table) | _PAGE_TABLE));
		BUG_ON(page_table != pte_offset_kernel(pde, 0));
	}

	/* PTE */
	pte = pte_offset_kernel(pde, vaddr);
	if (!pte)
		return NULL;

	set_pte_at(&init_mm, vaddr, pte, pfn_pte(pfn, PAGE_KERNEL));
	__flush_tlb_one_kernel(vaddr);
	return (void *)vaddr;
}

/* Destroy Page Table */
static void BiscuitOS_clear_page_table(unsigned long vaddr)
{
	pgd_t *pgd;
	pmd_t *pde;
	pte_t *pte;

	pgd = pgd_offset_k(vaddr);
	/* 32-bit Paging only contains PDE and PTE */
	pde = (pmd_t *)pgd;
	if (pmd_none(*pde) || unlikely(pmd_bad(*pde))) {
		printk("PDE doesn't exist!\n");
		return;
	}

	pte = pte_offset_kernel(pde, vaddr);
	if (!pte) {
		printk("PTE doesn't exist!\n");
		return;
	}

	/* Clear */
	pte_clear(&init_mm, vaddr, pte);
	__flush_tlb_one_kernel(vaddr);
}

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	unsigned long *addr;

	/* Build Page Table */
	addr = (unsigned long *)BiscuitOS_build_page_table(
			BISCUITOS_VADDR, BISCUITOS_BASE_PFN, BISCUITOS_SIZE);

	*addr = 88520;

	if (!pfn_valid(BISCUITOS_BASE_PFN)) {
		printk("\n\n\n***********BiscuitOS**********\n");
		printk("=> %#lx: %ld\n", (unsigned long)addr, *addr);
		printk("PFN Range: %#lx - %#lx\n", BISCUITOS_BASE_PFN,
			BISCUITOS_BASE_PFN + ((BISCUITOS_SIZE) >> PAGE_SHIFT));
		printk("max_pfn:   %#lx\n", max_pfn);
		printk("********************************\n\n\n");
	}

	/* Clear Page Table */
	BiscuitOS_clear_page_table((unsigned long)addr);

	/* Trigger kernel panic if access addr */
	//*addr = 88520;

	printk("Hello BiscuitOS\n");

	return 0;
}
device_initcall(BiscuitOS_init);
