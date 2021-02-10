/*
 * Translation 4K Page With Paging Mechanism on Kernel
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

#define BISCUITOS_KERNEL_VADDR		(VMALLOC_START + 0x700000)
#define BISCUITOS_KERNEL_SIZE		PAGE_SIZE

/* PTE: Page Table Entry */
static pte_t *BiscuitOS_pte_populate(pmd_t *pde, unsigned long addr)
{
	pte_t *pte = pte_offset_kernel(pde, addr);
	if (pte_none(*pte)) {
		struct page *page;
		pte_t entry;

		page = alloc_page(GFP_KERNEL);
		if (!page) {
			printk("Error: Memory in short on pte.\n");
			return NULL;
		}
		/* setup pte entry */
		entry = pfn_pte(page_to_pfn(page), PAGE_KERNEL);
		/* populate pte */
		set_pte_at(&init_mm, addr, pte, entry);
	}
	return pte;
}

/* PDE: Page Directory Entry */
static pmd_t *BiscuitOS_pde_populate(pud_t *pdt, unsigned long addr)
{
	pmd_t *pde = pmd_offset(pdt, addr);
	if (pmd_none(*pde)) {
		struct page *page;
		void *page_table;

		page = alloc_page(GFP_KERNEL);
		if (!page) {
			printk("Error: Memory in short on pmd.\n");
			return NULL;
		}
		page_table = page_address(page);
		/* pmd populate */
		pmd_populate_kernel(&init_mm, pde, page_table);
	}
	return pde;
}

/* Page Directory Table */
static pgd_t *BiscuitOS_pgd_populate(unsigned long addr)
{
	pgd_t *pgd = pgd_offset_k(addr);
	if (pgd_none(*pgd)) {
		struct page *page;
		void *page_table;

		/* Allocate new page for page table */
		page = alloc_page(GFP_KERNEL);
		if (!page) {
			printk("Error: Memory in short on pgd.\n");
			return NULL;
		}
		page_table = page_address(page);
		/* pgd populate */
		pgd_populate(&init_mm, pgd, page_table);
	}
	return pgd;
}

static int BiscuitOS_populate_page_table(unsigned long start,
						unsigned long end)
{
	pgd_t *pgd;
	pud_t *pdt;
	pmd_t *pde;
	pte_t *pte;

	/* BiscuitOS: Page for Directory Table */
	pgd = BiscuitOS_pgd_populate(start);
	if (!pgd)
		return -ENOMEM;

	/* PDT: Page Directory Table */
	pdt = (pud_t *)pgd;

	/* BiscuitOS: PDE - Page Directory Entry */
	pde = BiscuitOS_pde_populate(pdt, start);
	if (!pde)
		return -ENOMEM;

	/* BiscuitOS: PTE - Page Table Entry */
	pte = BiscuitOS_pte_populate(pde, start);
	if (!pte)
		return -ENOMEM;

	return 0;
}

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	unsigned long start = BISCUITOS_KERNEL_VADDR;
	unsigned long end = start + BISCUITOS_KERNEL_SIZE;
	unsigned long *val = (unsigned long *)start;

	/* Establish Kernel page table */
	BiscuitOS_populate_page_table(start, end);

	/* Trigger kernel page fault */
	*val = 88520;
	/* Using Virtual address */
	printk("\n\n\n*************BiscuitOS************\n");
	printk("Default Virtual Address: %#lx - %#lx\n", start, end);
	printk("BiscuitOS %#lx => %ld\n", (unsigned long)val, *val);

	printk("Hello BiscuitOS\n");
	printk("**********************************\n\n\n");

	return 0;
}
device_initcall(BiscuitOS_init);
