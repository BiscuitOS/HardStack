/*
 * Vmalloc Allocator
 *
 * (C) 2020.02.12 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linux/vmalloc.h"
#include "linux/buddy.h"
#include "linux/slub.h"
#include "linux/getorder.h"
#include "linux/rbtree.h"

static struct vmap_block_queue vmap_block_queue;
static struct vfree_deferred vfree_deferred;
static struct vm_struct *vmlist;
static unsigned long vmap_area_pcpu_hole;
static bool vmap_initialized = false;
pgprot_t pgprot_kernel;

#define VM_LAZY_FREE	0x02
#define VM_VM_AREA	0x04

/* The vmap cache globals are protected by vmap_area_lock */
static struct rb_node *free_vmap_cache;
static unsigned long cached_hole_size;
static unsigned long cached_vstart;
static unsigned long cached_align;

static unsigned long vmap_lazy_nr = 0;

static struct rb_root vmap_area_root = RB_ROOT;
LIST_HEAD(vmap_area_list);

struct mm_struct init_mm;

/* page-table-directory */
pgd_t *swapper_pg_dir;

void vmalloc_init(void)
{
	struct vmap_area *va;
	struct vm_struct *tmp;
	int i;

	for_each_possible_cpu(i) {
		struct vmap_block_queue *vbq;
		struct vfree_deferred *p;

		vbq = &vmap_block_queue;
		INIT_LIST_HEAD(&vbq->free);
		p = &vfree_deferred;
		init_llist_head(&p->list);
	}

	vmap_area_pcpu_hole = VMALLOC_END;
	vmap_initialized = true;
	pgprot_kernel = __pgprot(L_PTE_PRESENT | L_PTE_YOUNG | L_PTE_DIRTY);
	__create_page_table();
}

static void __insert_vmap_area(struct vmap_area *va)
{
	struct rb_node **p = &vmap_area_root.rb_node;
	struct rb_node *parent = NULL;
	struct rb_node *tmp;

	while (*p) {
		struct vmap_area *tmp_va;

		parent = *p;
		tmp_va = rb_entry(parent, struct vmap_area, rb_node);
		if (va->va_start < tmp_va->va_end)
			p = &(*p)->rb_left;
		else if (va->va_end > tmp_va->va_start)
			p = &(*p)->rb_right;
		else
			BUG();
	}

	rb_link_node(&va->rb_node, parent, p);
	rb_insert_color(&va->rb_node, &vmap_area_root);

	/* address-sort this list */
	tmp = rb_prev(&va->rb_node);
	if (tmp) {
		struct vmap_area *prev;

		prev = rb_entry(tmp, struct vmap_area, rb_node);
		list_add(&va->list, &prev->list);
	} else
		list_add(&va->list, &vmap_area_list);
}

static struct vmap_area *alloc_vmap_area(unsigned long size,
			unsigned long align, unsigned long vstart,
			unsigned long vend, int node, gfp_t gfp_mask)
{
	struct vmap_area *va;
	struct rb_node *n;
	unsigned long addr;
	int purged = 0;
	struct vmap_area *first;

	va = kmalloc_node(sizeof(struct vmap_area),
				gfp_mask & GFP_RECLAIM_MASK, node);
	if (unlikely(!va))
		return ERR_PTR(-ENOMEM);

retry:
	/*
	 * Invalidate cache if we have more permissive parameters.
	 * cached_hole_size notes the largest hole noticed _below_
	 * the vmap_area cached in free_vmap_cache: if size fits
	 * into that hole, we want to scan from vstart to reuse
	 * the hole instead of allocating above free_vmap_cache
	 * Note that __free_vmap_area may update free_vmap_cache
	 * without updating cached_hole_size or cached_align.
	 */
	if (!free_vmap_cache ||
		size < cached_hole_size ||
		vstart < cached_vstart ||
		align < cached_align) {
nocache:
		cached_hole_size = 0;
		free_vmap_cache = NULL;
	}
	/* record if we encounter less permissive parameters */
	cached_vstart = vstart;
	cached_align = align;

	/* find starting point for our search */
	if (free_vmap_cache) {
		first = rb_entry(free_vmap_cache, struct vmap_area, rb_node);
		addr = ALIGN(first->va_end, align);
		if (addr < vstart)
			goto nocache;
		if (addr + size < addr)
			goto overflow;
	} else {
		addr = ALIGN(vstart, align);
		if (addr + size < addr)
			goto overflow;

		n = vmap_area_root.rb_node;
		first = NULL;

		while (n) {
			struct vmap_area *tmp;

			tmp = rb_entry(n, struct vmap_area, rb_node);
			if (tmp->va_end >= addr) {
				first = tmp;
				if (tmp->va_start <= addr)
					break;
				n = n->rb_left;
			} else
				n = n->rb_right;
		}

		if (!first)
			goto found;
	}

	/* from the starting point, walk areas until a suitable hole is found */
	while (addr + size > first->va_start && addr + size <= vend) {
		if (addr + cached_hole_size < first->va_start)
			cached_hole_size = first->va_start - addr;
		addr = ALIGN(first->va_end, align);
		if (addr + size < addr)
			goto overflow;

		if (list_is_last(&first->list, &vmap_area_list))
			goto found;

		first = list_next_entry(first, list);
	}

found:
	if (addr + size > vend)
		goto overflow;

	va->va_start = addr;
	va->va_end = addr + size;
	va->flags = 0;
	__insert_vmap_area(va);
	free_vmap_cache = &va->rb_node;
	
	return va;
overflow:
	if (!purged) {
		printk("Need Lazy %s\n", __func__);
		goto retry;
	}
	kfree(va);
	return ERR_PTR(-EBUSY);
}

static void setup_vmalloc_vm(struct vm_struct *vm, struct vmap_area *va,
				unsigned long flags, const void *caller)
{
	vm->flags = flags;
	vm->addr = (void *)va->va_start;
	vm->size = va->va_end - va->va_start;
	vm->caller = caller;
	va->vm = vm;
	va->flags |= VM_VM_AREA;
}

static struct vm_struct *__get_vm_area_node(unsigned long size,
		unsigned long align, unsigned long flags, unsigned long start,
		unsigned long end, int node, gfp_t gfp_mask, const void *caller)
{
	struct vmap_area *va;
	struct vm_struct *area;

	size = PAGE_ALIGN(size);
	if (unlikely(!size))
		return NULL;

	if (flags & VM_IOREMAP)
		align = 1ul << clamp_t(int, get_count_order_long(size),
					PAGE_SHIFT, IOREMAP_MAX_ORDER);

	area = kzalloc_node(sizeof(*area), gfp_mask & GFP_RECLAIM_MASK, node);
	if (unlikely(!area))
		return NULL;

	if (!(flags & VM_NO_GUARD))
		size += PAGE_SIZE;

	va = alloc_vmap_area(size, align, start, end, node, gfp_mask);
	if (IS_ERR(va)) {
		kfree(area);
		return NULL;
	}

	setup_vmalloc_vm(area, va, flags, caller);

	return area;
}

static struct vmap_area *__find_vmap_area(unsigned long addr)
{
	struct rb_node *n = vmap_area_root.rb_node;

	while (n) {
		struct vmap_area *va;

		va = rb_entry(n, struct vmap_area, rb_node);
		if (addr < va->va_start)
			n = n->rb_left;
		else if (addr >= va->va_end)
			n = n->rb_right;
		else
			return va;
	}

	return NULL;
}

static struct vmap_area *find_vmap_area(unsigned long addr)
{
	struct vmap_area *va;

	va = __find_vmap_area(addr);

	return va;
}

static void vunmap_pte_range(pmd_t *pmd, unsigned long addr, unsigned long end)
{
	pte_t *pte;

	pte = pte_offset_kernel(pmd, addr);
	do {
		pte_t ptent = ptep_get_and_clear(&init_mm, addr, pte);
	} while (pte++, addr += PAGE_SIZE, addr != end);
}

static void vunmap_pmd_range(pud_t *pud, unsigned long addr, unsigned long end)
{
	pmd_t *pmd;
	unsigned long next;

	pmd = pmd_offset(pud, addr);
	do {
		next = pmd_addr_end(addr, end);
		if (pmd_clear_huge(pmd))
			continue;
		if (pmd_none_or_clear_bad(pmd))
			continue;
		vunmap_pte_range(pmd, addr, next);
	} while (pmd++, addr = next, addr != end);
}

static void vunmap_pud_range(pgd_t *pgd, unsigned long addr, unsigned long end)
{
	pud_t *pud;
	unsigned long next;

	pud = pud_offset(pgd, addr);
	do {
		next = pud_addr_end(addr, end);
		if (pud_clear_huge(pud))
			continue;
		if (pud_none_or_clear_bad(pud))
			continue;

		vunmap_pmd_range(pud, addr, next);
	} while (pud++, addr = next, addr != end);
}

static void vunmap_page_range(unsigned long addr, unsigned long end)
{
	pgd_t *pgd;
	unsigned long next;

	BUG_ON(addr >= end);
	pgd = pgd_offset_k(addr);
	do {
		next = pgd_addr_end(addr, end);
		if (pgd_none_or_clear_bad(pgd))
			continue;
		vunmap_pud_range(pgd, addr, next);
	} while (pgd++, addr = next, addr != end);

}

/*
 * Clear the pagetable entries of a given vmap_area
 */
static void unmap_vmap_area(struct vmap_area *va)
{
	vunmap_page_range(va->va_start, va->va_end);
}

static void __free_vmap_area(struct vmap_area *va)
{
	BUG_ON(RB_EMPTY_NODE(&va->rb_node));
	
	if (free_vmap_cache) {
		if (va->va_end < cached_vstart) {
			free_vmap_cache = NULL;
		} else {
			struct vmap_area *cache;
			cache = rb_entry(free_vmap_cache, struct vmap_area,
								rb_node);
			if (va->va_start <= cache->va_start) {
				free_vmap_cache = rb_prev(&va->rb_node);
				/*
				 * We don't try to update cached_hole_size or
				 * cached_align, but it won't go very wrong.
				 */
			}
		}
	}
	rb_erase(&va->rb_node, &vmap_area_root);
	RB_CLEAR_NODE(&va->rb_node);
	list_del(&va->list);

	/*
	 * Track the highest possible candidata for pcpu area
	 * allocation. Areas outside of vmalloc area can be returned
	 * here too, consider only end addresses which fall inside
	 * vmalloc area proper.
	 */
	if (va->va_end > VMALLOC_START && va->va_end <= VMALLOC_END)
		vmap_area_pcpu_hole = max(vmap_area_pcpu_hole, va->va_end);
	kfree(va);
}

/*
 * Free and unmap a vmap area
 */
static void free_unmap_vmap_area(struct vmap_area *va)
{
	unmap_vmap_area(va);

	/* no lazy free, come on now! */
	__free_vmap_area(va);
}

/*
 * remove_vm_area - find and remove a continuous kernel virtual area.
 */
struct vm_struct *remove_vm_area(const void *addr)
{
	struct vmap_area *va;

	va = find_vmap_area((unsigned long)addr);
	if (va && va->flags & VM_VM_AREA) {
		struct vm_struct *vm = va->vm;

		va->vm = NULL;
		va->flags &= ~VM_VM_AREA;
		va->flags |= VM_LAZY_FREE;

		free_unmap_vmap_area(va);
		return vm;
	}
	return NULL;
}

int __pte_alloc_kernel(pmd_t *pmd)
{
	pte_t *new = pte_alloc_one_kernel(&init_mm);
	if (!new)
		return -ENOMEM;

	if (likely(pmd_none(*pmd))) { /* Has another populated it ? */
		pmd_populate_kernel(&init_mm, pmd, new);
		new = NULL;
	}
	if (new)
		pte_free_kernel(&init_mm, new);
	return 0;
}

static int vmap_pte_range(pmd_t *pmd, unsigned long addr,
		unsigned long end, pgprot_t prot, struct page **pages, int *nr)
{
	pte_t *pte;

	/*
	 * nr is a running index into the array which helps higher level
	 * callers keep track of where we're up to.
	 */
	pte = pte_alloc_kernel(pmd, addr);
	if (!pte)
		return -ENOMEM;
	do {
		struct page *page = pages[*nr];

		if (!pte_none(*pte))
			return -EBUSY;
		if (!page)
			return -ENOMEM;
		set_pte_at(&init_mm, addr, pte, mk_pte(page, prot));
		(*nr)++;
	} while (pte++, addr += PAGE_SIZE, addr != end);
	return 0;
}

static int vmap_pmd_range(pud_t *pud, unsigned long addr,
		unsigned long end, pgprot_t prot, struct page **pages, int *nr)
{
	pmd_t *pmd;
	unsigned long next;

	pmd = pmd_alloc(&init_mm, pud, addr);
	if (!pmd)
		return -ENOMEM;
	do {
		next = pmd_addr_end(addr, end);
		if (vmap_pte_range(pmd, addr, next, prot, pages, nr))
			return -ENOMEM;
	} while (pmd++, addr = next, addr != end);
	return 0;
}

static int vmap_pud_range(pgd_t *pgd, unsigned long addr,
		unsigned long end, pgprot_t prot, struct page **pages, int *nr)
{
	pud_t *pud;
	unsigned long next;

	pud = pud_alloc(&init_mm, pgd, addr);
	if (!pud)
		return -ENOMEM;
	do {
		next = pud_addr_end(addr, end);
		if (vmap_pmd_range(pud, addr, next, prot, pages, nr))
			return -ENOMEM;
	} while (pud++, addr = next, addr != end);
	return 0;
}

/*
 * Set up page tables in kva (addr, end). The ptes shall have prot "prot", and
 * will have pfns corresponding to the "pages" array.
 *
 * Ie. pte at addr+N*PAGE_SIZE shall point to pfn corresponding to pages[N]
 */
static int vmap_page_range_noflush(unsigned long start, unsigned long end,
			pgprot_t prot, struct page **pages)
{
	pgd_t *pgd;
	unsigned long next;
	unsigned long addr = start;
	int err = 0;
	int nr = 0;

	BUG_ON(addr >= end);
	pgd = pgd_offset_k(addr);
	do {
		next = pgd_addr_end(addr, end);
		err = vmap_pud_range(pgd, addr, next, prot, pages, &nr);
		if (err)
			return err;
	} while (pgd++, addr = next, addr != end);

	return nr;
}

static int vmap_page_range(unsigned long start, unsigned long end,
				pgprot_t prot, struct page **pages)
{
	int ret;

	ret = vmap_page_range_noflush(start, end, prot, pages);
	return ret;
}

int map_vm_area(struct vm_struct *area, pgprot_t prot, struct page **pages)
{
	unsigned long addr = (unsigned long)area->addr;
	unsigned long end = addr + get_vm_area_size(area);
	int err;

	err = vmap_page_range(addr, end, prot, pages);

	return err > 0 ? 0 : err;
}

void kvfree(const void *addr)
{
	kfree(addr);
}

static void __vunmap(const void *addr, int deallocate_pages)
{
	struct vm_struct *area;

	if (!addr)
		return;

	if (!PAGE_ALIGNED(addr)) {
		printk("Trying to vfree() bad address (%p)\n", addr);
		return;
	}

	area = find_vmap_area((unsigned long)addr)->vm;
	if (unlikely(!area)) {
		printk("Trying to vfree() nonexistent vm area (%p)\n", addr);
		return;
	}

	remove_vm_area(addr);
	if (deallocate_pages) {
		int i;

		for (i = 0; i < area->nr_pages; i++) {
			struct page *page = area->pages[i];

			BUG_ON(!page);
			__free_pages(page, 0);
		}

		kvfree(area->pages);
	}

	kfree(area);
	return;
}

/*
 * vfree - release memory allocated by vmalloc()
 */
void vfree(const void *addr)
{
	if (!addr)
		return;
	__vunmap(addr, 1);
}

static void *__vmalloc_area_node(struct vm_struct *area, gfp_t gfp_mask,
				pgprot_t prot, int node)
{
	struct page **pages;
	unsigned int nr_pages, array_size, i;
	const gfp_t nested_gfp = (gfp_mask & GFP_RECLAIM_MASK) | __GFP_ZERO;
	const gfp_t alloc_mask = gfp_mask | __GFP_NOWARN;
	const gfp_t highmem_mask = (gfp_mask & (GFP_DMA | GFP_DMA32)) ?
						0 :
						__GFP_HIGHMEM;

	nr_pages = get_vm_area_size(area) >> PAGE_SHIFT;
	array_size = (nr_pages * sizeof(struct page *));

	area->nr_pages = nr_pages;
	/* Please note that the recursion is strictly bounded. */
	if (array_size > PAGE_SIZE) {
		pages = __vmalloc_node(array_size, 1, nested_gfp|highmem_mask,
				PAGE_KERNEL, node, area->caller);
	} else {
		pages = kmalloc_node(array_size, nested_gfp, node);
	}
	area->pages = pages;
	if (!area->pages) {
		remove_vm_area(area->addr);
		kfree(area);
		return NULL;
	}

	for (i = 0; i < area->nr_pages; i++) {
		struct page *page;

		page = alloc_page(alloc_mask | highmem_mask);
		if (unlikely(!page)) {
			/* Successfully allocated i pages, free them in __vunmap() */
			area->nr_pages = i;
			goto fail;
		}
		area->pages[i] = page;
	}

	if (map_vm_area(area, prot, pages))
		goto fail;
	return area->addr;

fail:
	printk("vmalloc: allocation failure, allocated %#lx of %#lx bytes\n",
			(unsigned long)(area->nr_pages * PAGE_SIZE), 
			(unsigned long)area->size);
	vfree(area->addr);
	return NULL;
}

static void clear_vm_uninitialized_flag(struct vm_struct *vm)
{
	vm->flags &= ~VM_UNINITIALIZED;
}

void *__vmalloc_node_range(unsigned long size, unsigned long align,
		unsigned long start, unsigned long end, gfp_t gfp_mask,
		pgprot_t prot, unsigned long vm_flags, int node,
		const void *caller)
{
	struct vm_struct *area;
	void *addr;
	unsigned long real_size = size;

	size = PAGE_ALIGN(size);
	if (!size || (size >> PAGE_SHIFT) > totalram_pages())
		goto fail;

	area = __get_vm_area_node(size, align, VM_ALLOC | VM_UNINITIALIZED |
			vm_flags, start, end, node, gfp_mask, caller);
	if (!area)
		goto fail;

	addr = __vmalloc_area_node(area, gfp_mask, prot, node);
	if (!addr)
		return NULL;

	/*
	 * In this function, newly allocated vm_struct has VM_UNINITIALIZED
	 * flags. It means that vm_struct is not fully initialized.
	 * Now, it is fully initialized, so remove this flag here.
	 */
	clear_vm_uninitialized_flag(area);

	return addr;

fail:
	return NULL;
}

/*
 * __vmalloc_node - allocate virtually contiguous memory
 */
static void *__vmalloc_node(unsigned long size, unsigned long align,
		gfp_t gfp_mask, pgprot_t prot,
		int node, const void *caller)
{
	return __vmalloc_node_range(size, align, VMALLOC_START, VMALLOC_END,
			gfp_mask, prot, 0, node, caller);
}

static inline void *__vmalloc_node_flags(unsigned long size,
				int node, gfp_t flags)
{
	return __vmalloc_node(size, 1, flags, PAGE_KERNEL,
				node, __builtin_return_address(0));
}

/*
 * vmalloc - allocate virtually contiguous memory
 * @size:	allocation size
 * Allocate enough pages to cover @size from the page level
 * allocator and map them into contiguous kernel virtual space.
 */
void *vmalloc(unsigned long size)
{
	return __vmalloc_node_flags(size, NUMA_NO_NODE,
					GFP_KERNEL);
}

/* Establish page table on boot stage */
void __create_page_table(void)
{
	swapper_pg_dir = (pgd_t *)kmalloc(PG_DIR_SIZE, GFP_KERNEL);
	/* clear page table */
	memset(swapper_pg_dir, 0, PG_DIR_SIZE);
	init_mm.pgd = swapper_pg_dir;
	printk("PAGE-Table-Directory: %#lx - %#lx\n", 
			(unsigned long)swapper_pg_dir,
			(unsigned long)swapper_pg_dir + PG_DIR_SIZE);
	printk("VMALLOC Page-Direct:  %#lx - %#lx\n", 
			(unsigned long)pgd_offset_k(VMALLOC_START),
			(unsigned long)pgd_offset_k(VMALLOC_END));
}

/* Emulate paging */
unsigned long *mmu_vaddr_to_addr(unsigned long vaddr)
{
	unsigned long addr;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	pgd = pgd_offset_k(vaddr);
	if (pgd_none(*pgd)) {
		printk("BUG(): %#lx none pgd entry\n", vaddr);
		return NULL;
	}

	pud = pud_offset(pgd, vaddr);
	if (pud_none(*pud)) {
		printk("BUG(): %#lx none pud entry\n", vaddr);
		return NULL;
	}

	pmd = pmd_offset(pud, vaddr);
	if (pmd_none(*pmd)) {
		printk("BUG(): %#lx none pmd entry\n", vaddr);
		return NULL;
	}

	pte = pte_offset_kernel(pmd, vaddr);
	if (pte_none(*pte)) {
		printk("BUG(): %#lx none pte entry\n", vaddr);
		return NULL;
	}
	/* set_pte_at */
	addr = pte_val(*pte) & PAGE_MASK;
	addr |= vaddr & PAGE_UMASK;
	return phys_to_virt(addr);
}

/* dup RB-TREE */
void dup_RBTREE(void)
{
	struct rb_node *node;

        /* dup RB-tree */
	for(node = rb_first(&vmap_area_root); node; node = rb_next(node)) {
		struct vmap_area *va;

		va = rb_entry(node, struct vmap_area, rb_node);
		printk("RB AREA: %#lx - %#lx\n", va->va_start, va->va_end);
	}
}
