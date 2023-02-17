/*
 * BiscuitOS VMALLOC TO Userspace
 *
 * (C) 2023.02.18 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.04.01 BiscuitOS
 *                <https://biscuitos.github.io/blog/BiscuitOS_Catalogue/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sysctl.h>
#include <linux/vmalloc.h>
#include <asm/pgalloc.h>

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

static pte_t *BiscuitOS_populate_page_table(unsigned long start)
{
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	/* BiscuitOS pgd */
	pgd = BiscuitOS_pgd_populate(start);
	if (!pgd)
		return NULL;

	/* BiscuitOS p4d */
	p4d = BiscuitOS_p4d_populate(pgd, start);
	if (!p4d)
		return NULL;

	/* BiscuitOS pud */
	pud = BiscuitOS_pud_populate(p4d, start);
	if (!pud)
		return NULL;

	/* BiscuitOS pmd */
	pmd = BiscuitOS_pmd_populate(pud, start);
	if (!pmd)
		return NULL;

	/* BiscuitOS pte */
	pte = BiscuitOS_pte_populate(pmd, start);
	if (!pte)
		return NULL;

	return pte;
}

SYSCALL_DEFINE0(BiscuitOS_valloc)
{
	struct vm_struct *area;
	struct page *page;
	pgprot_t prot;
	pte_t *pte;

	area = get_vm_area(PAGE_SIZE, VM_USERMAP);
	if (!area) {
		printk("System Error: no free VMALLOC memory.\n");
		return -ENOMEM;
	}

	page = alloc_page(GFP_USER);
	if (!page) {
		printk("System Error: no free Buddy memory.\n");
		free_vm_area(area);
		return -ENOMEM;
	}

	pte = BiscuitOS_populate_page_table((unsigned long)area->addr);
	if (!pte) {
		printk("System: Invalid pte\n");
	}

	pgprot_val(prot) = _PAGE_USER | _PAGE_PRESENT | _PAGE_RW; 
	
	get_page(page);
	set_pte(pte, mk_pte(page, prot));

	return (unsigned long)area->addr;
}

SYSCALL_DEFINE1(BiscuitOS_vfree, void *, addr)
{
	/* vfree */
	struct vm_struct *area;

	area = find_vm_area(addr);
	if (!area) {
		printk("System Error: Unknow address!\n");
		return -EINVAL;
	}

	free_vm_area(area);
	return 0;
}
