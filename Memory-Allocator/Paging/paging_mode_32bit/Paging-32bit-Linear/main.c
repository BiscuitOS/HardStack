/*
 * Linear Mapping with 32-bit Paging
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

/* Linear Mapping */
#define BISCUITOS_va(x)		((void *)((unsigned long)(x)+PAGE_OFFSET))
#define BISCUITOS_pa(x)		((x) - PAGE_OFFSET)

static int BiscuitOS_follow_page_table(unsigned long vaddr, pte_t **ptep)
{
	pgd_t *pgd;
	pmd_t *pde;
	pte_t *pte;

	pgd = pgd_offset_k(vaddr);
	/* On 32-bit paging contains PDE and PTE */
	pde = (pmd_t *)pgd;
	if (pmd_none(*pde) || unlikely(pmd_bad(*pde)))
		return -EINVAL;

	pte = pte_offset_kernel(pde, vaddr);
	if (!pte || !pte_present(*pte))
		return -EINVAL;

	*ptep = pte;
	return 0;
}

static int __init BiscuitOS_init(void)
{
	phys_addr_t phys, _phys, phys_follow;
	unsigned long pfn, pfn_follow;
	unsigned long vaddr, _vadr;
	struct page *page;
	pte_t *pte;

	page = alloc_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;

	/* Address information from page */
	vaddr = (unsigned long)page_address(page);
	pfn   = page_to_pfn(page);
	phys  = page_to_phys(page);

	/* Linear Mapping */
	_vadr = (unsigned long)BISCUITOS_va(phys);
	_phys = BISCUITOS_pa(vaddr);

	/* Follow page table */
	if (BiscuitOS_follow_page_table(vaddr, &pte))
		return -EINVAL;

	/* Address information from page table */
	pfn_follow  = pte_pfn(*pte);
	phys_follow = PFN_PHYS(pfn_follow);

	printk("\n\n\n**********BiscuitOS***********\n");
	printk("Default:\n");
	printk("  Virtual Address:  %#lx\n", vaddr);
	printk("  Physical Address: %#lx\n", (unsigned long)phys);
	printk("Linear Mapping:\n");
	printk("  Virtual Address:  %#lx\n", _vadr);
	printk("  Physical Address: %#lx\n", (unsigned long)_phys);
	printk("Follow Page Table:\n");
	printk("  Physical PFN:     %#lx\n", pfn_follow);
	printk("  Physical Address: %#lx\n", (unsigned long)phys_follow);
	printk("\n");
	printk("Linear Range: 0 - %#lx\n", (unsigned long)PFN_PHYS(max_pfn));
	printk("********************************\n\n\n");

	__free_page(page);

	printk("Hello BiscuitOS\n");

	return 0;
}
device_initcall(BiscuitOS_init);
