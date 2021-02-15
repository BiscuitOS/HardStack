/*
 * Temporary kernel mappings (KMAP) With 32-Bit Paging
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

/* The pte for KMAP */
static pte_t *BiscuitOS_kmap_pte;

static void __BiscuitOS_kunmap_atomic(void *kvaddr);

#define BiscuitOS_kunmap_atomic(addr)				\
do {								\
	BUILD_BUG_ON(__same_type((addr), struct page *));	\
	__BiscuitOS_kunmap_atomic(addr);			\
} while (0)

/* Clear a kernel PTE and flush it from the TLB */
#define BiscuitOS_kpte_clear_flush(ptep, vaddr)			\
do {								\
	pte_clear(&init_mm, (vaddr), (ptep));			\
	__flush_tlb_one_kernel((vaddr));			\
} while (0)

/* 
 * kmap_atomic/kunmap_atomic is significantly faster then kmap/kunmap because
 * no global lock is needed and because the kmap code must perform a global
 * TLB invalidation when the kmap pool wraps.
 *
 * However when holding an atomic kmap it is not legal to sleep, so atomic
 * kmaps are appropriate for short, tight code paths only.
 */
static void *BiscuitOS_kmap_atomic_prot(struct page *page, pgprot_t prot)
{
	unsigned long vaddr;
	int idx, type;

	preempt_disable();
	pagefault_disable();

	if (!PageHighMem(page))
		return page_address(page);

	type = kmap_atomic_idx_push();
	idx = type + KM_TYPE_NR * smp_processor_id();
	vaddr = __fix_to_virt(FIX_KMAP_BEGIN + idx);
	BUG_ON(!pte_none(*(BiscuitOS_kmap_pte - idx)));
	set_pte(BiscuitOS_kmap_pte - idx, mk_pte(page, prot));
	arch_flush_lazy_mmu_mode();

	return (void *)vaddr;
}

static void *BiscuitOS_kmap_atomic(struct page *page)
{
	return BiscuitOS_kmap_atomic_prot(page, kmap_prot);
}

static inline pte_t *kmap_get_fixmap_pte(unsigned long vaddr)
{
	pgd_t *pgd = pgd_offset_k(vaddr);
	pmd_t *pde = (pmd_t *)pgd;
	return pte_offset_kernel(pde, vaddr);
}

static void __init kmap_init(void)
{
	unsigned long kmap_vstart;

	/* Cache the first kmap pte */
	kmap_vstart = __fix_to_virt(FIX_KMAP_BEGIN);
	BiscuitOS_kmap_pte = kmap_get_fixmap_pte(kmap_vstart);
}

static void __BiscuitOS_kunmap_atomic(void *kvaddr)
{
	unsigned long vaddr = (unsigned long)kvaddr & PAGE_MASK;

	if (vaddr >= __fix_to_virt(FIX_KMAP_END) &&
	    vaddr <= __fix_to_virt(FIX_KMAP_BEGIN)) {
		int idx, type;

		type = kmap_atomic_idx();
		idx = type + KM_TYPE_NR * smp_processor_id();

		/*
		 * Force other mappings to Oops if they'll try to access this
		 * pte without first remap it. Keeping stale mappings around
		 * is a bad idea also, in case the page changes cacheability
		 * attributes or becomes a protected page in a hypervisor.
		 */
		BiscuitOS_kpte_clear_flush(BiscuitOS_kmap_pte - idx, vaddr);
		kmap_atomic_idx_pop();
		arch_flush_lazy_mmu_mode();
	}

	pagefault_enable();
	preempt_enable();
}

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	struct page *page;
	unsigned long *addr;

	/* Kmap init */
	kmap_init();

	page = alloc_page(GFP_KERNEL | __GFP_HIGHMEM);
	if (!page)
		return -ENOMEM;

	/* KMAP Mapping */
	addr = (unsigned long *)BiscuitOS_kmap_atomic(page);

	/* Safe to use */
	*addr = 88520;

	printk("\n\n\n\n*************BiscuitOS************\n");
	printk("=> %#lx: %ld\n", (unsigned long)addr, *addr);
	printk("BiscuitOS KMAP Memory Allocator.\n");
	printk("*********************************\n\n\n\n");

	/* KMAP Un-Mapping */
	BiscuitOS_kunmap_atomic(addr);

	/* Trigger kernel panic if access addr */
	//*addr = 88520;

	__free_page(page);
	return 0;
}
device_initcall(BiscuitOS_init);
