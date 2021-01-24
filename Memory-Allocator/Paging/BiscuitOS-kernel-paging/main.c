/*
 * Paging Mechanism on Kernel
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

/* PTE */
static pte_t *BiscuitOS_pte_populate(pmd_t *pmd, unsigned long addr)
{
	pte_t *pte = pte_offset_kernel(pmd, addr);
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

/* PMD */
static pmd_t *BiscuitOS_pmd_populate(pud_t *pud, unsigned long addr)
{
	pmd_t *pmd = pmd_offset(pud, addr);
	if (pmd_none(*pmd)) {
		struct page *page;
		void *page_table;

		page = alloc_page(GFP_KERNEL);
		if (!page) {
			printk("Error: Memory in short on pmd.\n");
			return NULL;
		}
		page_table = page_address(page);
		/* pmd populate */
		pmd_populate_kernel(&init_mm, pmd, page_table);
	}
	return pmd;
}

/* PUD */
static pud_t *BiscuitOS_pud_populate(p4d_t *p4d, unsigned long addr)
{
	pud_t *pud = pud_offset(p4d, addr);
	if (pud_none(*pud)) {
		struct page *page;
		void *page_table;

		/* Allocate new page for page table */
		page = alloc_page(GFP_KERNEL);
		if (!page) {
			printk("Error: Memory in short on pud.\n");
			return NULL;
		}
		page_table = page_address(page);
		/* pud populate */
		pud_populate(&init_mm, pud, page_table);
	}

	return pud;
}

/* P4D */
static p4d_t *BiscuitOS_p4d_populate(pgd_t *pgd, unsigned long addr)
{
	p4d_t *p4d = p4d_offset(pgd, addr);
	if (p4d_none(*p4d)) {
		struct page *page;
		void *page_table;

		/* Allocate new page for page table */
		page = alloc_page(GFP_KERNEL);
		if (!page) {
			printk("Error: Memory in short on p4d\n");
			return NULL;
		}
		page_table = page_address(page);
		/* p4d populate */
		p4d_populate(&init_mm, p4d, page_table);
	}
	return p4d;
}

/* PGD */
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
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	/* BiscuitOS pgd */
	pgd = BiscuitOS_pgd_populate(start);
	if (!pgd)
		return -ENOMEM;

	/* BiscuitOS p4d */
	p4d = BiscuitOS_p4d_populate(pgd, start);
	if (!p4d)
		return -ENOMEM;

	/* BiscuitOS pud */
	pud = BiscuitOS_pud_populate(p4d, start);
	if (!pud)
		return -ENOMEM;

	/* BiscuitOS pmd */
	pmd = BiscuitOS_pmd_populate(pud, start);
	if (!pmd)
		return -ENOMEM;

	/* BiscuitOS pte */
	pte = BiscuitOS_pte_populate(pmd, start);
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
	printk("Default Virtual Address: %#lx - %#lx\n", start, end);
	printk("BiscuitOS %#lx => %#lx\n", (unsigned long)val, *val);

	printk("Hello modules on BiscuitOS\n");

	return 0;
}
device_initcall(BiscuitOS_init);
