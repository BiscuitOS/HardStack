/*
 * High-Memory
 *
 * (C) 2020.02.18 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "linux/buddy.h"
#include "linux/highmem.h"
#include "linux/pgtable.h"
#include "linux/slub.h"

#define PA_HASH_ORDER		7

static int pkmap_count[LAST_PKMAP];
pte_t *pkmap_page_table;
pgprot_t pgprot_kernel;

/*
 * Describes one page->virtual association
 */
struct page_address_map {
	struct page *page;
	void *virtual;
	struct list_head list;
};

static struct page_address_map page_address_maps[LAST_PKMAP];

/*
 * Hash table bucket
 */
static struct page_address_slot {
	struct list_head lh;	/* List of page_address_maps */
} page_address_htable[1 << PA_HASH_ORDER];

static struct page_address_slot *page_slot(const struct page *page)
{
	return &page_address_htable[hash_ptr(page, PA_HASH_ORDER)];
}

/*
 * page_address - get the mapped virtual address of a page
 */
void *page_address(const struct page *page)
{
	unsigned long flags;
	void *ret;
	struct page_address_slot *pas;

	if (!PageHighMem(page))
		return lowmem_page_address(page);

	pas = page_slot(page);
	ret = NULL;
	if (!list_empty(&pas->lh)) {
		struct page_address_map *pam;

		list_for_each_entry(pam, &pas->lh, list) {
			if (pam->page == page) {
				return pam->virtual;
				goto done;
			}
		}
	}
done:
	return ret;
}

void page_address_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(page_address_htable); i++) {
		INIT_LIST_HEAD(&page_address_htable[i].lh);
	}
	pgprot_kernel = __pgprot(L_PTE_PRESENT | L_PTE_YOUNG | L_PTE_DIRTY);
}

/**
 * set_page_address - set a page's virtual address
 */
void set_page_address(struct page *page, void *virtual)
{
	unsigned long flags;
	struct page_address_slot *pas;
	struct page_address_map *pam;

	if (!PageHighMem(page)) {
		printk("BUG_ON(): need high-memory %s\n", __func__);
		return;
	}

	pas = page_slot(page);
	if (virtual) {	/* Add */
		pam = &page_address_maps[PKMAP_NR((unsigned long)virtual)];
		pam->page = page;
		pam->virtual = virtual;

		list_add_tail(&pam->list, &pas->lh);
	} else {	/* Remove */
		list_for_each_entry(pam, &pas->lh, list) {
			if (pam->page == page) {
				list_del(&pam->list);
				goto done;
			}
		}
	}
done:
	return;
}

static void flush_all_zero_pkmaps(void)
{
	int i;
	int need_flush = 0;

	printk("flush... %s\n", __func__);
	for (i = 0; i < LAST_PKMAP; i++) {
		struct page *page;

		/*
		 * zero means we don't have anything to do,
		 * >1 means that it is still in use. Only
		 * a count of 1 means that it is free but
		 * needs to be unmapped
		 */
		if (pkmap_count[i] != 1)
			continue;
		pkmap_count[i] = 0;

		/*
		 * Don't need an atomic fetch-and-clear op here;
		 * no-one has the page mapped, and cannot get at
		 * its virtual address (and hence PTE) without first
		 * getting the kmap_lock (which is held here).
		 * So no dangers, even with speculative execution.
		 */
		page = pte_page(pkmap_page_table[i]);
		pte_clear(&init_mm, PKMAP_ADDR(i), &pkmap_page_table[i]);

		set_page_address(page, NULL);
	}
}

static inline unsigned long map_new_virtual(struct page *page)
{
	unsigned long vaddr;
	int count;
	unsigned int last_pkmap_nr;
	unsigned int color = get_pkmap_color(page);

start:
	count = get_pkmap_entries_count(color);
	/* Find an empty entry */
	for (;;) {
		last_pkmap_nr = get_next_pkmap_nr(color);
		if (no_more_pkmaps(last_pkmap_nr, color)) {
			flush_all_zero_pkmaps();
			count = get_pkmap_entries_count(color);
		}
		if (!pkmap_count[last_pkmap_nr])
			break; /* Found a usable entry */
		if (--count)
			continue;

		/*
		 * Emulate sleep
		 */
		{
			if (page_address(page))
				return (unsigned long)page_address(page);
			/* Re-start */
			goto start;
		}
	}
	vaddr = PKMAP_ADDR(last_pkmap_nr);
	set_pte_at(&init_mm, vaddr,
		&(pkmap_page_table[last_pkmap_nr]), mk_pte(page, kmap_prot));

	pkmap_count[last_pkmap_nr] = 1;
	set_page_address(page, (void *)vaddr);

	return vaddr;
}

/*
 * kmap_high - map a highmem page into memory
 */
void *kmap_high(struct page *page)
{
	unsigned long vaddr;

	vaddr = (unsigned long)page_address(page);
	if (!vaddr)
		vaddr = map_new_virtual(page);
	pkmap_count[PKMAP_NR(vaddr)]++;
	if (pkmap_count[PKMAP_NR(vaddr)] < 2)
		printk("BUG_ON(): pkmap_count < 2\n");
	return (void *)vaddr;
}

/**
 * kunmap_high - unmap a highmem page into memory
 */
void kunmap_high(struct page *page)
{
	unsigned long vaddr;
	unsigned long nr;
	unsigned long flags;
	int need_wakeup;
	unsigned int color = get_pkmap_color(page);

	vaddr = (unsigned long)page_address(page);
	if (!vaddr) {
		printk("BUG_ON(): %s page_address() failed.\n", __func__);
		return;
	}
	nr = PKMAP_NR(vaddr);

	if (--pkmap_count[nr] == 0) {
		printk("BUG() pkmap_count[] %s\n", __func__);
		return;
	}
}

static inline void set_fixmap_pte(int idx, pte_t pte)
{
	unsigned long vaddr = __fix_to_virt(idx);
	pte_t *ptep = pte_offset_kernel(pmd_off_k(vaddr), vaddr);

	pmd_t *pmd = pmd_off_k(FIXADDR_START);
	set_pte_ext(ptep, pte);
}

/*
 * kmap_high_get - pin a highmem page into memory
 */
void *kmap_high_get(struct page *page)
{
	unsigned long vaddr, flags;

	vaddr = (unsigned long)page_address(page);
	if (vaddr) {
		if (pkmap_count[PKMAP_NR(vaddr)] < 1)
			printk("BUG_ON(): pkmap count < 1\n");
		pkmap_count[PKMAP_NR(vaddr)]++;
	}
	return (void *)vaddr;
}

void *kmap(struct page *page)
{
	if (!PageHighMem(page))
		return page_address(page);
	return kmap_high(page);
}

void kunmap(struct page *page)
{
	if (!PageHighMem(page))
		return;
	kunmap_high(page);
}

void *kmap_atomic(struct page *page)
{
	unsigned int idx;
	unsigned long vaddr;
	void *kmap;
	int type;

	if (!PageHighMem(page))
		return page_address(page);

	kmap = kmap_high_get(page);
	if (kmap)
		return kmap;

	type = kmap_atomic_idx_push();

	idx = FIX_KMAP_BEGIN + type + KM_TYPE_NR * 1;
	vaddr = __fix_to_virt(idx);
	/*
	 * When debugging is off, kunmap_atomic leaves the previous mapping
	 * in place, so the contained TLB flush ensures the TLB is updated
	 * with the new mapping.
	 */
	set_fixmap_pte(idx, mk_pte(page, kmap_prot));

	return (void *)vaddr;
}

void __kunmap_atomic(void *kvaddr)
{
	unsigned long vaddr = (unsigned long)kvaddr & PAGE_MASK;
	int idx, type;

	if (kvaddr >= (void *)FIXADDR_START) {
		type = kmap_atomic_idx();
		idx = FIX_KMAP_BEGIN + type + KM_TYPE_NR * 1;
		kmap_atomic_idx_pop();
	} else if (vaddr >= PKMAP_ADDR(0) && vaddr < PKMAP_ADDR(LAST_PKMAP)) {
		/* this address was obtained through kmap_high_get() */
		kunmap_high(pte_page(pkmap_page_table[PKMAP_NR(vaddr)]));
	}
}

static pte_t *early_pte_alloc(pmd_t *pmd, unsigned long addr,
				unsigned long prot)
{
	if (pmd_none(*pmd)) {
		pte_t *pte = kmalloc(PTE_HWTABLE_OFF + PTE_HWTABLE_SIZE,
						GFP_KERNEL);
		memset(pte, 0, PTE_HWTABLE_OFF + PTE_HWTABLE_SIZE);
		__pmd_populate(pmd, __pa(pte), prot);
	}
	return pte_offset_kernel(pmd, addr);
}

static inline pmd_t *fixmap_pmd(unsigned long addr)
{
	pgd_t *pgd = pgd_offset_k(addr);
	pud_t *pud = pud_offset(pgd, addr);
	pmd_t *pmd = pmd_offset(pud, addr);

	return pmd;
}

static pte_t bm_pte[PTRS_PER_PTE + PTE_HWTABLE_PTRS];

void fixmap_init(void)
{
	pmd_t *pmd;

	if ((__fix_to_virt(__end_of_early_ioremap_region) >> PMD_SHIFT) !=
			FIXADDR_TOP >> PMD_SHIFT);

	pmd = fixmap_pmd(FIXADDR_TOP);
	pmd_populate_kernel(&init_mm, pmd, bm_pte);
}

void kmap_init(void)
{
	/* FIXMAP */
	fixmap_init();
	/* KMAP */
	pkmap_page_table = early_pte_alloc(pmd_off_k(PKMAP_BASE),
			PKMAP_BASE, _PAGE_KERNEL_TABLE);
	/* FIXMAP */
	early_pte_alloc(pmd_off_k(FIXADDR_START), FIXADDR_START,
				    _PAGE_KERNEL_TABLE);
	printk("KMAP AREA:    %#lx - %#lx\n", PKMAP_BASE, 
				(unsigned long)MMU_PAGE_OFFSET);
	printk("FIXMAP AREA:  %#lx - %#lx\n", FIXADDR_START, FIXADDR_END);
}

void *kaddr_to_vaddr(void *kaddr)
{
	pte_t *ptep;
	
	if ((unsigned long)kaddr > FIXADDR_START) /* FIXMAP */
		ptep = pte_offset_kernel(pmd_off_k((unsigned long)kaddr), 
							(unsigned long)kaddr);
	else {/* KMAP */
		ptep = &pkmap_page_table[PKMAP_NR((unsigned long)kaddr)];
	}

	/* Check pte presetn */
	if (!pte_none(*ptep))
		return phys_to_virt(pte_val(*ptep) & PAGE_MASK);
	else
		return NULL;
}
