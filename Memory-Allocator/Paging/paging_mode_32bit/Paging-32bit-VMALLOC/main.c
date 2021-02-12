/*
 * VMALLOC for 4K Page With 32-Bit Paging
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
#include <linux/slab.h>
/* mm/internal.h */
#include "../mm/internal.h"

#define BISCUITOS_KERNEL_VADDR		(VMALLOC_START + 0x700000)
#define BISCUITOS_KERNEL_SIZE		PAGE_SIZE

#define VM_LAZY_FREE			0x02
#define VM_VM_AREA			0x04

/* vm_struct */
static struct vm_struct *BiscuitOS_area;

static int vmap_pte_range(pmd_t *pde, unsigned long addr,
		unsigned long end, pgprot_t prot, struct page **pages, int *nr)
{
	pte_t *pte;

	/*
	 * nr is a rnuning index into the array which helps higher level
	 * callers keep track of where we're up to.
	 */
	pte = pte_alloc_kernel(pde, addr);
	if (!pte)
		return -ENOMEM;
	do {
		struct page *page = pages[*nr];

		if (WARN_ON(!pte_none(*pte)))
			return -EBUSY;
		if (WARN_ON(!page))
			return -ENOMEM;
		set_pte_at(&init_mm, addr, pte, mk_pte(page, prot));
		(*nr)++;
	} while (pte++, addr += PAGE_SIZE, addr != end);
	return 0;
}

static int vmap_page_range_noflush(unsigned long start, unsigned long end,
					pgprot_t prot, struct page **pages)
{
	pgd_t *pgd;
	unsigned long next;
	unsigned long addr = start;
	int nr = 0;

	BUG_ON(addr >= end);
	pgd = pgd_offset_k(addr);
	do {
		pmd_t *pde;
		/* 4K Page With 32-Bit Paging only PDE and PTE */
		pde = (pmd_t *)pgd;

		next = pgd_addr_end(addr, end);
		if (vmap_pte_range(pde, addr, next, prot, pages, &nr))
			return -ENOMEM;
	} while (pgd++, addr = next, addr != end);

	return nr;
}

static int vmap_page_range(unsigned long start, unsigned long end, 
					pgprot_t prot, struct page **pages)
{
	int ret;

	ret = vmap_page_range_noflush(start, end , prot, pages);
	flush_cache_vmap(start, end);
	return ret;
}

static int BiscuitOS_map_vm_area(struct vm_struct *area,
				pgprot_t prot, struct page **pages)
{
	unsigned long addr = (unsigned long)area->addr;
	unsigned long end  = addr + get_vm_area_size(area);
	int err;

	err = vmap_page_range(addr, end, prot, pages);

	return err > 0 ? 0 : err;
}

static void *__vmalloc_area_node(struct vm_struct *area, gfp_t gfp_mask,
					pgprot_t prot, int node)
{
	struct page **pages;
	unsigned int nr_pages, array_size, i;
	const gfp_t nested_gfp = (gfp_mask & GFP_RECLAIM_MASK) | __GFP_ZERO;
	const gfp_t alloc_mask = gfp_mask | __GFP_NOWARN;
	const gfp_t highmem_mask = (gfp_mask & (GFP_DMA | GFP_DMA32)) ?
					0 : __GFP_HIGHMEM;

	nr_pages = get_vm_area_size(area) >> PAGE_SHIFT;
	array_size = (nr_pages * sizeof(struct page *));

	area->nr_pages = nr_pages;
	/* Please note that the recursion is strictly bounded. */
	pages = kmalloc_node(array_size, nested_gfp, node);
	area->pages = pages;

	/* Allocate Physical Memory */
	for (i = 0; i < area->nr_pages; i++) {
		struct page *page;

		if (node == NUMA_NO_NODE)
			page = alloc_page(alloc_mask | highmem_mask);
		else
			page = alloc_pages_node(node, alloc_mask|highmem_mask, 0);
		if (unlikely(!page)) {
			/* Successfuly allocated i page, free them
			 * in __vunmap()
			 */
			area->nr_pages = i;
			goto fail;
		}
		area->pages[i] = page;
	}

	/* Build Page */
	if (BiscuitOS_map_vm_area(area, prot, pages))
		goto fail;
	return area->addr;

fail:
	return NULL;
}

static void vunmap_pte_range(pmd_t *pde, unsigned long addr, unsigned long end)
{
	pte_t *pte;

	pte = pte_offset_kernel(pde, addr);
	do {
		pte_t ptent = ptep_get_and_clear(&init_mm, addr, pte);
		WARN_ON(!pte_none(ptent) && !pte_present(ptent));
	} while (pte++, addr += PAGE_SIZE, addr != end);
}

static void vunmap_page_range(unsigned long addr, unsigned long end)
{
	pgd_t *pgd;
	unsigned long next;

	BUG_ON(addr >= end);
	pgd = pgd_offset_k(addr);
	do {
		pmd_t *pde;

		/* 4K Page With 32-Bit Paging only contain PDE and PTE */
		pde = (pmd_t *)pgd;

		next = pgd_addr_end(addr, end);
		if (pgd_none_or_clear_bad(pgd))
			continue;
		vunmap_pte_range(pde, addr, next);
	} while (pgd++, addr = next, addr != end);
}

/* Clear the pagetable entries of a given vmap_area */
static void unmap_vmap_area(struct vmap_area *va)
{
	vunmap_page_range(va->va_start, va->va_end);
}

/* Free and unmap a vmap area */
static void free_unmap_vmap_area(struct vmap_area *va)
{
	flush_cache_vunmap(va->va_start, va->va_end);
	unmap_vmap_area(va);
	if (debug_pagealloc_enabled())
		flush_tlb_kernel_range(va->va_start, va->va_end);
}

static struct vm_struct *BiscuitOS_remove_vm_area(const void *addr)
{
	struct vmap_area *va;

	might_sleep();
	va->va_start = (unsigned long)BiscuitOS_area->addr;
	va->va_end   = (unsigned long)BiscuitOS_area->size + va->va_start;
	va->flags    = VM_LAZY_FREE;

	free_unmap_vmap_area(va);
	return BiscuitOS_area;
}

static void __vunmap(const void *addr)
{
	struct vm_struct *area;
	int i;

	area = BiscuitOS_remove_vm_area(addr);
	for (i = 0; i < area->nr_pages; i++) {
		struct page *page = area->pages[i];

		BUG_ON(!page);
		__free_pages(page, 0);

	}
	kvfree(area->pages);
	kfree(area);
}

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	unsigned long start = BISCUITOS_KERNEL_VADDR;
	unsigned long end = start + BISCUITOS_KERNEL_SIZE;
	struct vm_struct *area;
	unsigned long *addr;

	/* Setup VMALLOC area */
	area = kzalloc(sizeof(struct vm_struct), GFP_KERNEL);

	area->addr = (void *)start;
	area->size = end - start;
	area->flags = VM_ALLOC | VM_UNINITIALIZED | VM_NO_GUARD;
	/* for simple on BiscuitOS */
	BiscuitOS_area = area;

	/* Trigger kernel panic if access addr */
	// *addr = 88520;

	/* Emulate VMALLOC */
	addr = (unsigned long *)__vmalloc_area_node(area, 
				GFP_KERNEL, PAGE_KERNEL, NUMA_NO_NODE);
	if (!addr)
		return -EINVAL;

	/* used */
	*addr = 88520;

	printk("\n\n\n\n***************BiscuitOS**************\n");
	printk("=> %#lx: %ld\n", (unsigned long)addr, *addr);
	printk("VMALLOC Range: %#lx - %#lx\n", start, end);
	printk("**************************************\n\n\n\n");

	/* unsed */
	__vunmap(addr);
	/* Trigger kernel panic if execute here */
	//*addr = 88520;

	printk("Hello BiscuitOS\n");

	return 0;
}
device_initcall(BiscuitOS_init);
