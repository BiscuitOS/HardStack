/*
 * Permanent kamp Allocator (PKMAP) With 32-Bit Paging
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
#include <linux/wait.h>

/* BiscuitOS Emulate PKMAP Area */
#define BISCUITOS_PKMAP_BASE		((PAGE_OFFSET - 0x1800000) & PMD_MASK)
#define BISCUITOS_PKMAP_NR(virt)	((virt - BISCUITOS_PKMAP_BASE) >> \
							PAGE_SHIFT)
#define BISCUITOS_PKMAP_ADDR(nr)	(BISCUITOS_PKMAP_BASE + \
							((nr) << PAGE_SHIFT))

/* KMAP lock */
static __cacheline_aligned_in_smp DEFINE_SPINLOCK(kmap_lock);
/* pkmap_count */
static int pkmap_count[LAST_PKMAP];
/* PTE for PKMAP */
pte_t *BiscuitOS_pkmap_page_table;

#define lock_kmap()		spin_lock(&kmap_lock)
#define unlock_kmap()		spin_unlock(&kmap_lock)

static inline int get_pkmap_entries_count(unsigned int color)
{
	return LAST_PKMAP;
}

static inline unsigned int get_next_pkmap_nr(unsigned int color)
{
	static unsigned int last_pkmap_nr;

	last_pkmap_nr = (last_pkmap_nr + 1) & LAST_PKMAP_MASK;
	return last_pkmap_nr;
}

static inline int no_more_pkmaps(unsigned int pkmap_nr, unsigned int color)
{
	return pkmap_nr == 0;
}

static void flush_all_zero_pkmaps(void)
{
	int i;
	int need_flush = 0;

	flush_cache_kmaps();

	for (i = 0; i < LAST_PKMAP; i++) {
		struct page *page;

		if (pkmap_count[i] != 1)
			continue;
		pkmap_count[i] = 0;

		/* sanity check */
		BUG_ON(pte_none(BiscuitOS_pkmap_page_table[i]));

		page = pte_page(BiscuitOS_pkmap_page_table[i]);
		pte_clear(&init_mm, PKMAP_ADDR(i), 
					&BiscuitOS_pkmap_page_table[i]);

		set_page_address(page, NULL);
		need_flush = 1;
	}
	if (need_flush)
		flush_tlb_kernel_range(PKMAP_ADDR(0), PKMAP_ADDR(LAST_PKMAP));
}

static inline wait_queue_head_t *get_pkmap_wait_queue_head(unsigned int color)
{
	static DECLARE_WAIT_QUEUE_HEAD(pkmap_map_wait);

	return &pkmap_map_wait;
}

static inline unsigned long map_new_virtual(struct page *page)
{
	unsigned long vaddr;
	int count;
	unsigned last_pkmap_nr;
	unsigned int color = 0;

start:
	count = get_pkmap_entries_count(color);
	/* Find an empty entry */
	for (;;) {
		last_pkmap_nr = get_next_pkmap_nr(color);
		if (!no_more_pkmaps(last_pkmap_nr, color)) {
			flush_all_zero_pkmaps();
			count = get_pkmap_entries_count(color);
		}
		if (!pkmap_count[last_pkmap_nr])
			break; /* Found a usable entry */
		if (--count)
			continue;

		{
			DECLARE_WAITQUEUE(wait, current);
			wait_queue_head_t *pkmap_map_wait = 
				get_pkmap_wait_queue_head(color);

			__set_current_state(TASK_UNINTERRUPTIBLE);
			add_wait_queue(pkmap_map_wait, &wait);
			unlock_kmap();
			schedule();
			remove_wait_queue(pkmap_map_wait, &wait);
			lock_kmap();

			if (page_address(page))
				return (unsigned long)page_address(page);

			goto start;
		}
	}
	vaddr = BISCUITOS_PKMAP_ADDR(last_pkmap_nr);
	set_pte_at(&init_mm, vaddr, 
			&(BiscuitOS_pkmap_page_table[last_pkmap_nr]), 
						mk_pte(page, kmap_prot));

	pkmap_count[last_pkmap_nr] = 1;
	set_page_address(page, (void *)vaddr);

	return vaddr;
}

static void *BiscuitOS_kmap_high(struct page *page)
{
	unsigned long vaddr;

	lock_kmap();
	vaddr = (unsigned long)page_address(page);
	if (!vaddr)
		vaddr = map_new_virtual(page);
	pkmap_count[BISCUITOS_PKMAP_NR(vaddr)]++;
	BUG_ON(pkmap_count[BISCUITOS_PKMAP_NR(vaddr)] < 2);
	unlock_kmap();
	return (void *)vaddr;
}

static void BiscuitOS_kunmap_high(struct page *page)
{
	wait_queue_head_t *pkmap_map_wait;
	unsigned int color = 0;
	unsigned long vaddr;
	unsigned long nr;
	int need_wakeup;

	lock_kmap();
	vaddr = (unsigned long)page_address(page);
	BUG_ON(!vaddr);
	nr = BISCUITOS_PKMAP_NR(vaddr);

	/*
	 * A count must never go down to zero
	 * without a TLB flush!
	 */
	need_wakeup = 0;
	switch (--pkmap_count[nr]) {
	case 0:
		BUG();
	case 1:
		/*
		 * Avoid an unnecessary wake_up() function call.
		 * The common case is pkmap_count[] == 1, but
		 * no waiters.
		 * The tasks queued in the wait-queue are guarded
		 * by both the lock in the wait-queue-head and by
		 * the kmap_lock.  As the kmap_lock is held here,
		 * no need for the wait-queue-head's lock.  Simply
		 * test if the queue is empty.
		 */
		pkmap_map_wait = get_pkmap_wait_queue_head(color);
		need_wakeup = waitqueue_active(pkmap_map_wait);
	}
	unlock_kmap();

	/* do wake-up, if needed, race-free outside of the spin lock */
	if (need_wakeup)
		wake_up(pkmap_map_wait);
}

static void *BiscuitOS_kmap(struct page *page)
{
	might_sleep();
	if (!PageHighMem(page))
		return page_address(page);
	return BiscuitOS_kmap_high(page);
}

static void BiscuitOS_kunmap(struct page *page)
{
	if (in_interrupt())
		BUG();
	if (!PageHighMem(page))
		return;
	BiscuitOS_kunmap_high(page);
}

static inline void *alloc_low_page(void)
{
	struct page *page = alloc_page(GFP_KERNEL);

	return page_address(page);
}

static void __init
page_table_range_init(unsigned long start, unsigned long end, pgd_t *pgd_base)
{
	int pgd_idx, pde_idx;
	unsigned long vaddr;
	pgd_t *pgd;
	pmd_t *pde;

	vaddr = start;
	pgd_idx = pgd_index(vaddr);
	pde_idx = pmd_index(vaddr);
	pgd = pgd_base + pgd_idx;

	for ( ; (pgd_idx < PTRS_PER_PGD) && (vaddr != end); pgd++, pgd_idx++) {
		/* 4K Page With 32-Bit Paging only contains PDE and PTE */
		pde = (pmd_t *)pgd + pde_idx;

		if (!(pmd_val(*pde) & _PAGE_PRESENT)) {
			pte_t *page_table = (pte_t *)alloc_low_page();

			paravirt_alloc_pte(&init_mm, 
					__pa(page_table) >> PAGE_SHIFT);
			set_pmd(pde, __pmd(__pa(page_table) | _PAGE_TABLE));
			BUG_ON(page_table != pte_offset_kernel(pde, 0));

			vaddr += PMD_SIZE;
		}
	}
}

static void __init permanent_kmaps_init(void)
{
	pgd_t *pgd_base = swapper_pg_dir;
	unsigned long vaddr;
	pgd_t *pgd;
	pmd_t *pde;
	pte_t *pte;

	vaddr = BISCUITOS_PKMAP_BASE;
	page_table_range_init(vaddr, vaddr + PAGE_SIZE * LAST_PKMAP, pgd_base);

	pgd = swapper_pg_dir + pgd_index(vaddr);
	pde = (pmd_t *)pgd;
	pte = pte_offset_kernel(pde, vaddr);
	BiscuitOS_pkmap_page_table = pte;
}

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	struct page *page;
	unsigned long *addr;

	/* permanent kamp init */
	permanent_kmaps_init();

	page = alloc_page(GFP_KERNEL | __GFP_HIGHMEM);
	if (!page)
		return -ENOMEM;

	/* PKMAP mapping */
	addr = (unsigned long *)BiscuitOS_kmap(page);

	*addr = 88520;
	printk("\n\n\n\n****************BiscuitOS**************\n");
	printk("=> %#lx: %ld\n", (unsigned long)addr, *addr);
	printk("***************************************\n\n\n\n");

	/* PKMAP un-mapping */
	BiscuitOS_kunmap(page);

	__free_page(page);
	return 0;
}
device_initcall(BiscuitOS_init);
