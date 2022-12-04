#ifndef _BISCUITOS_HIGHMEM_H
#define _BISCUITOS_HIGHMEM_H

#include "linux/pgtable.h"
#include "linux/buddy.h"

#define KM_TYPE_NR	16
#define NR_CPUS		1

enum fixed_addresses { 
	FIX_EARLYCON_MEM_BASE,
	__end_of_permanent_fixed_addresses,

	FIX_KMAP_BEGIN = __end_of_permanent_fixed_addresses,
	FIX_KMAP_END = FIX_KMAP_BEGIN + (KM_TYPE_NR * NR_CPUS) - 1,

	/* Support writing RO kernel text via kprobes, jump labels, etc. */
	FIX_TEXT_POKE0,
	FIX_TEXT_POKE1,

	__end_of_fixmap_region,

	/*      
	 * Share the kmap() region with early_ioremap(): this is guaranteed
	 * not to clash since early_ioremap() is only available before
	 * paging_init(), and kmap() only after.
	 */ 
#define NR_FIX_BTMAPS           32
#define FIX_BTMAPS_SLOTS        7
#define TOTAL_FIX_BTMAPS        (NR_FIX_BTMAPS * FIX_BTMAPS_SLOTS)

	FIX_BTMAP_END = __end_of_permanent_fixed_addresses,
	FIX_BTMAP_BEGIN = FIX_BTMAP_END + TOTAL_FIX_BTMAPS - 1,
	__end_of_early_ioremap_region
};

/* PKMAP */
#define PKMAP_BASE		(MMU_PAGE_OFFSET - PMD_SIZE)
#define LAST_PKMAP		PTRS_PER_PTE
#define LAST_PKMAP_MASK		(LAST_PKMAP - 1)
#define PKMAP_NR(virt)		(((virt) - PKMAP_BASE) >> PAGE_SHIFT)
#define PKMAP_ADDR(nr)		(PKMAP_BASE + ((nr) << PAGE_SHIFT))

/* FIXMAP */
#define FIXADDR_START		0xffc00000UL
#define FIXADDR_END		0xfff00000UL
#define FIXADDR_TOP		(FIXADDR_END - PAGE_SIZE)

#define GOLDEN_RATIO_32	0x61C88647

static inline unsigned int __hash_32(unsigned int val)
{
	return val * GOLDEN_RATIO_32;
}

static inline unsigned int hash_32(unsigned int val, unsigned int bits)
{
	return __hash_32(val) >> (32 - bits);
}

#define hash_long(val, bits)	hash_32(val, bits)

static inline unsigned int hash_ptr(const void *ptr, unsigned int bits)
{
	return hash_long((unsigned long)ptr, bits);
}

static inline int PageHighMem(const struct page *page)
{
	if (page_zone(page) == &BiscuitOS_zone)
		return 0;
	else
		return 1;
}

/*
 * Determine color of virtual address where the page should be mapped.
 */
static inline unsigned int get_pkmap_color(struct page *page)
{
	return 0;
}

/*
 * Get the number of PKMAP entries of the given color. If no free slot is
 * found after checking that many entries.
 */
static inline int get_pkmap_entries_count(unsigned int color)
{
	return LAST_PKMAP;
}

/*
 * Get next index for mapping inside PKMAP region for page with given color.
 */
static inline unsigned int get_next_pkmap_nr(unsigned int color)
{
	static unsigned int last_pkmap_nr;

	last_pkmap_nr = (last_pkmap_nr + 1) & LAST_PKMAP_MASK;
	return last_pkmap_nr;
}

/*
 * Determine if page index inside PKMAP region (pkmap_nr) of given color
 * has wrapped around PKMAP region end. When this happens an attempt to
 * flush all unused PKMAP slots is made.
 */
static inline int no_more_pkmaps(unsigned int pkmap_nr, unsigned int color)
{
	return pkmap_nr == 0;
}

static int __kmap_atomic_idx;

static inline int kmap_atomic_idx_push(void)
{
	int idx = ++__kmap_atomic_idx - 1;

	return idx;
}

static inline int kmap_atomic_idx(void)
{
	return __kmap_atomic_idx - 1;
}

static inline void kmap_atomic_idx_pop(void)
{
	__kmap_atomic_idx--;
}

extern void __kunmap_atomic(void *kvaddr);

/*
 * Prevent perple trying to call kunmap_atomic() as if it were kunmap()
 * kunmap_atomic() should get the return value of kmap_atomic, not the page.
 */
#define kunmap_atomic(addr)					\
do {								\
	__kunmap_atomic(addr);					\
} while (0)

/* Fixmap */
#define __fix_to_virt(x)	(FIXADDR_TOP - ((x) << PAGE_SHIFT))
#define __virt_to_fix(x)	((FIXADDR_TOP - ((x)&PAGE_MASK)) >> PAGE_SHIFT)

extern void *kmap(struct page *page);
extern void *kmap_atomic(struct page *page);
extern void *page_address(const struct page *page);
extern void kmap_init(void);
extern void kunmap(struct page *page);
extern void *kaddr_to_vaddr(void *kaddr);
#endif
