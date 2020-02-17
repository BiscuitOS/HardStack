#ifndef _BISCUITOS_BUDDY_H
#define _BISCUITOS_BUDDY_H

#include "linux/list.h"
#include "linux/gfp.h"
#include "linux/biscuitos.h"
#include "linux/bitmap.h"

#define PAGE_SHIFT	12 /* 4KByte Page */
#define PAGE_SIZE	(1 << PAGE_SHIFT)
#define PAGE_MASK	(~((1 << PAGE_SHIFT) - 1))
#define PAGE_UMASK	(PAGE_SIZE - 1)
#define PAGE_ALIGN(addr) ALIGN(addr, PAGE_SIZE)
#define PFN_ALIGN(x)	(((unsigned long)(x) + (PAGE_SIZE - 1)) & PAGE_MASK)
#define PFN_UP(x)	(((x) + PAGE_SIZE-1) >> PAGE_SHIFT)
#define PFN_DOWN(x)	((x) >> PAGE_SHIFT)
#define PFN_PHYS(x)	((phys_addr_t)(x) << PAGE_SHIFT)
#define PHYS_PFN(x)	((unsigned long)(x) >> PAGE_SHIFT)

#define IS_ALIGNED(x, a)	(((x) & ((typeof(x))(a) - 1)) == 0)
#define PAGE_ALIGNED(addr)	IS_ALIGNED((unsigned long)(addr), PAGE_SIZE)

/* Configuration Memory Region */
#define MEMORY_SIZE	CONFIG_MEMORY_SIZE
/* Configuration Basic Physical Address */
#define PHYS_OFFSET	CONFIG_PHYS_BASE
/* Configuration PAGE_OFFSET */
#define PAGE_OFFSET	(PHYS_OFFSET + (unsigned long)CONFIG_PAGE_OFFSET)

#define PFN_OFFSET	PHYS_PFN(PHYS_OFFSET)
#define PHYS_MASK	(~0UL)

typedef u32 pteval_t;
typedef u32 pmdval_t;

/*
 * These are used to make use of C type-checking
 */
typedef struct { pteval_t pte; } pte_t;
typedef struct { pmdval_t pmd; } pmd_t;
typedef struct { pmdval_t pgd[2]; } pgd_t;
typedef struct { pteval_t pgprot; } pgprot_t;
typedef struct { pgd_t pgd; } pud_t;

#define pte_val(x)	((x).pte)
#define pmd_val(x)	((x).pmd)
#define pgd_val(x)	((x).pgd[0])
#define pgprot_val(x)	((x).pgprot)

#define __pte(x)	((pte_t) { (x) } )
#define __pmd(x)	((pmd_t) { (x) } )
#define __pgprot(x)	((pgprot_t) { (x) } )

/*
 * Hardware-wise, we have a two level page table structure, where the first
 * level has 4096 entries, and the second level has 256 entries. Each entry
 * is one 32-bit word. Most of the bits in the second level entry are used
 * by hardware, and there aren't any "accessed" and "dirty" bits.
 *
 * Linux on the other hand has a three level page table structure, which can
 * be wrapped to fit a two level page table structure easily - using the PGD
 * and PTE only. However, Linux also expects one "PTE" table per page, and
 * at least a "dirty" bit.
 *
 * Therefore, we tweak the implementation slightly - we tell Linux that we
 * have 2048 entries in the first level, each of which is 8 bytes (iow, two
 * hardware pointers to the second level.) The second level contains two
 * hardware PTE table arranged contiguously, preceded by Linux versions
 * which contain the state information Linux needs. We, therefore, end up
 * with 512 entries in the "PTE" level.
 *
 * This leads to the page tables having the following layout:
 *
 *     pgd
 * |        |        +------------+ +0
 * +--------+        | Linux pt 0 |
 * |        |        +------------+ +1024
 * +--------+ +0     | Linux pt 1 |
 * |        | -----> +------------+ +2048
 * +--------+ +4     |  h/w pt 0  |
 * |        | -----> +------------+ +3072
 * +--------+ +8     |  h/w pt 1  |
 * |        |        +------------+ +4096
 * +--------+
 */
#define PTRS_PER_PTE		512
#define PTRS_PER_PMD		1
#define PTRS_PER_PGD		2048

#define PTE_HWTABLE_PTRS	(PTRS_PER_PTE)
#define PTE_HWTABLE_OFF		(PTE_HWTABLE_PTRS * sizeof(pte_t))
#define PTE_HWTABLE_SIZE	(PTRS_PER_PTE * sizeof(u32))

/*
 * PMD_SHIFT determines the size of the area a second-level page table can
 * map PGDIR_SHIFT determines what a third-level page table entry can map.
 */
#define PMD_SHIFT		21
#define PGDIR_SHIFT		21

#define PMD_SIZE		(1UL << PMD_SHIFT)
#define PMD_MASK		(~(PMD_SIZE-1))
#define PGDIR_SIZE		(1UL << PGDIR_SHIFT)
#define PGDIR_MASK		(~(PGDIR_SIZE-1))

#define PUD_SHIFT		PGDIR_SHIFT
#define PUD_SIZE		(1UL << PUD_SHIFT)
#define PUD_MASK		(~(PUD_SIZE-1))

/*
 * Hardware page table definitions
 *
 * + Level 1 descriptor (PMD)
 *   - common
 */
#define PMD_TYPE_MASK		(_AT(pmdval_t, 3) << 0)
#define PMD_TYPE_FAULT		(_AT(pmdval_t, 0) << 0)
#define PMD_TYPE_TABLE		(_AT(pmdval_t, 1) << 0)
#define PMD_TYPE_SECT		(_AT(pmdval_t, 2) << 0)
#define PMD_PXNTABLE		(_AT(pmdval_t, 1) << 2)         /* v7 */
#define PMD_BIT4		(_AT(pmdval_t, 1) << 4)
#define PMD_DOMAIN(x)		(_AT(pmdval_t, (x)) << 5)

/*
 * - section
 */
#define PMD_SECT_PXN		(_AT(pmdval_t, 1) << 0)         /* v7 */
#define PMD_SECT_BUFFERABLE	(_AT(pmdval_t, 1) << 2)
#define PMD_SECT_CACHEABLE	(_AT(pmdval_t, 1) << 3)
#define PMD_SECT_XN		(_AT(pmdval_t, 1) << 4)         /* v6 */
#define PMD_SECT_AP_WRITE	(_AT(pmdval_t, 1) << 10)
#define PMD_SECT_AP_READ	(_AT(pmdval_t, 1) << 11)
#define PMD_SECT_TEX(x)		(_AT(pmdval_t, (x)) << 12)      /* v5 */
#define PMD_SECT_APX		(_AT(pmdval_t, 1) << 15)        /* v6 */
#define PMD_SECT_S		(_AT(pmdval_t, 1) << 16)        /* v6 */
#define PMD_SECT_nG		(_AT(pmdval_t, 1) << 17)        /* v6 */
#define PMD_SECT_SUPER		(_AT(pmdval_t, 1) << 18)        /* v6 */
#define PMD_SECT_AF		(_AT(pmdval_t, 0))

#define PMD_SECT_UNCACHED	(_AT(pmdval_t, 0))
#define PMD_SECT_BUFFERED	(PMD_SECT_BUFFERABLE)
#define PMD_SECT_WT		(PMD_SECT_CACHEABLE)
#define PMD_SECT_WB		(PMD_SECT_CACHEABLE | PMD_SECT_BUFFERABLE)
#define PMD_SECT_MINICACHE	(PMD_SECT_TEX(1) | PMD_SECT_CACHEABLE)
#define PMD_SECT_WBWA		(PMD_SECT_TEX(1) | PMD_SECT_CACHEABLE | \
                                                   PMD_SECT_BUFFERABLE)
#define PMD_SECT_CACHE_MASK	(PMD_SECT_TEX(1) | PMD_SECT_CACHEABLE | \
                                                   PMD_SECT_BUFFERABLE)
#define PMD_SECT_NONSHARED_DEV	(PMD_SECT_TEX(2))
#define PMD_FLAGS_UP		PMD_SECT_WB

#define _PAGE_KERNEL_TABLE	(PMD_TYPE_TABLE | PMD_BIT4 | \
                                 PMD_DOMAIN(DOMAIN_KERNEL))

#define _PAGE_USER_TABLE	(PMD_TYPE_TABLE | PMD_BIT4 | \
                                 PMD_DOMAIN(DOMAIN_KERNEL))
/*
 * section address mask and size definitions.
 */
#define SECTION_SHIFT		20
#define SECTION_SIZE		(1UL << SECTION_SHIFT)
#define SECTION_MASK		(~(SECTION_SIZE-1))

/*
 * "Linux" PTE definitions
 *
 * We keep two sets of PTEs - the hardware and the linux version.
 * This allow greater flexiblilty in the way we map the Linux bits
 * onto the hardware tables, and allows us to have YOUNG and DIRTY
 * bits.
 *
 * The PTE table pointer refers to the hardware entries: the "Linux"
 * entries are stored 1024 bytes below.
 */
#define L_PTE_VALID		(_AT(pteval_t, 1) << 0)         /* Valid */
#define L_PTE_PRESENT		(_AT(pteval_t, 1) << 0)
#define L_PTE_YOUNG		(_AT(pteval_t, 1) << 1)
#define L_PTE_DIRTY		(_AT(pteval_t, 1) << 6)
#define L_PTE_RDONLY		(_AT(pteval_t, 1) << 7)
#define L_PTE_USER		(_AT(pteval_t, 1) << 8)
#define L_PTE_XN		(_AT(pteval_t, 1) << 9)
#define L_PTE_SHARED		(_AT(pteval_t, 1) << 10)
#define L_PTE_NONE		(_AT(pteval_t, 1) << 11)

#define L_PTE_MT_DEV_SHARED	(_AT(pteval_t, 0x04) << 2)
#define L_PTE_MT_MASK		(_AT(pteval_t, 0x0f) << 2)

#define _MOD_PROT(p, b)		__pgprot(pgprot_val(p) | (b))
extern pgprot_t			pgprot_kernel;

#define DOMAIN_KERNEL		2

#define PAGE_KERNEL		_MOD_PROT(pgprot_kernel, L_PTE_XN)
#define _PAGE_KERNEL_TABLE	(PMD_TYPE_TABLE | PMD_BIT4 | \
				 PMD_DOMAIN(DOMAIN_KERNEL))

/*** Page-table ***/
extern struct mm_struct init_mm;

/* to find an entry in a page-table-directory */
#define pgd_index(addr)		((addr) >> PGDIR_SHIFT)

#define pgd_offset(mm, addr)	((mm)->pgd + pgd_index(addr))

/* to find an entry in a kernel page-table-directory */
#define pgd_offset_k(addr)	pgd_offset(&init_mm, addr)

#define pgd_addr_end(addr, end)						\
({	unsigned long __boundary = ((addr) + PGDIR_SIZE) & PGDIR_MASK;	\
	(__boundary - 1 < (end) - 1)? __boundary : (end);		\
})

#define pud_addr_end(addr, end)						\
({	unsigned long __boundary = ((addr) + PUD_SIZE) & PUD_MASK;	\
	(__boundary - 1 < (end) - 1)? __boundary : (end);		\
})

#define pmd_addr_end(addr, end)						\
({	unsigned long __boundary = ((addr) + PMD_SIZE) & PMD_MASK;	\
	(__boundary - 1 < (end) - 1)? __boundary : (end);		\
})

static inline int pgd_none(pgd_t pgd)		{ return 0; }
static inline int pgd_bad(pgd_t pgd)		{ return 0; }
static inline int pgd_present(pgd_t pgd)	{ return 1; }
static inline void pgd_clear(pgd_t *pgd)	{ }

static inline void pgd_clear_bad(pgd_t *pgd)
{
	pgd_clear(pgd);
}

static inline int pgd_none_or_clear_bad(pgd_t *pgd)
{
	if (pgd_none(*pgd))
		return 1;
	if (unlikely(pgd_bad(*pgd))) {
		pgd_clear_bad(pgd);
		return 1;
	}
	return 0;
}

#define pud_none(pud)		(0)
#define pud_bad(pud)		(0)
#define pud_present(pud)	(1)
#define pud_clear(pudp)		do {} while (0)
#define set_pud(pud,pudp)	do {} while (0)

static inline pud_t *pud_offset(pgd_t *pgd, unsigned long address)
{
	return (pud_t *)pgd;
}

static inline int pud_clear_huge(pud_t *pud)
{
	return 0;
}

static inline int pud_none_or_clear_bad(pud_t *pud)
{
	if (pud_none(*pud))
		return 1;
	if (unlikely(pud_bad(*pud))) {
		pud_clear(pud);
		return 1;
	}
	return 0;
}

#define pmd_none(pmd)		(!pmd_val(pmd))
#define pmd_bad(pmd)		(pmd_val(pmd) & 2)
#define pmd_large(pmd)		(pmd_val(pmd) & 2)
#define pmd_present(pmd)	(pmd_val(pmd))

#define pmd_clear(pmdp)				\
	do {					\
		pmdp[0] = __pmd(0);		\
		pmdp[1] = __pmd(0);		\
	} while (0)

static inline pmd_t *pmd_offset(pud_t *pud, unsigned long addr)
{
	return (pmd_t *)pud;
}

static inline int pmd_clear_huge(pmd_t *pmd)
{
	return 0;
}

static inline int pmd_none_or_clear_bad(pmd_t *pmd)
{
	if (pmd_none(*pmd))
		return 1;
	if (unlikely(pmd_bad(*pmd))) {
		pmd_clear(pmd);
		return 1;
	}
	return 0;
}

#define PG_DIR_SIZE		0x4000

/* Buddy Allocator Order */
#define MAX_ORDER	11

/* Emulate printk information */
#define printk(...)	printf(__VA_ARGS__)

typedef unsigned long phys_addr_t;

struct free_area {
	struct list_head free_list[1];
	unsigned long nr_free;
};

struct zone {
	/* free areas of different sizes */
	struct free_area free_area[MAX_ORDER];
};

struct page {
	unsigned long flags;	/* Atomic flags, some possible updated async */
	union {
		struct {
			struct list_head lru;
			unsigned long private;
		};
		struct {
			union {
				struct list_head slab_list;
				struct {
					struct page *next;
#ifdef CONFIG_64BIT
					int pages;
					int pobjects;
#else
					short int pages;
					short int pobjects;
#endif
				};
			};
			struct kmem_cache *slab_cache; /* not slob */
			/* Double-word boundary */
			void *freelist;		/* first free object */
			union {
				void *s_mem;	/* slab: first object */
				unsigned long counters;	/* SLUB */
				/*
		 		* 32       31            16                0
		 		* +--------+-------------+-----------------+
		 		* | frozen |   objects   |      inuse      |
		 		* +--------+-------------+-----------------+
		 		*/
				struct {
					unsigned inuse:16;
					unsigned objects:15;
					unsigned frozen:1;
				};
			};
		};
		struct {	/* Tail pages of compound page */
			unsigned long compound_head;	/* Bit zero is set */

			/* First tail page only */
			unsigned char compound_dtor;
			unsigned char compound_order;
			unsigned long compound_mapcount;
		};
	};

	union {		/* This union is 4 bytes in size. */
		unsigned int page_type;
	};
	/* Usage count. */
	unsigned long _refcount;
};

extern struct page *mem_map;

/* PFN and PHYS */
#define pfn_to_page(pfn)	(mem_map + ((pfn) - PFN_OFFSET))
#define page_to_pfn(page)	((unsigned long)((page) - mem_map) + \
							PFN_OFFSET)

#define pfn_valid_within(pfn)	(1)

struct mm_struct {
	struct {
		pgd_t *pgd;
	};
};

/* Physical and Virtual */
extern unsigned char *memory;
static inline phys_addr_t virt_to_phys(const volatile void *x)
{
	return ((unsigned long)(x) - (unsigned long)memory) + 
							PHYS_OFFSET;
}

static inline void *phys_to_virt(phys_addr_t x)
{
	return (void *)((x - PHYS_OFFSET) + (unsigned long)memory);
}

static inline unsigned long virt_to_pfn(const volatile void *x)
{
	return PHYS_PFN((unsigned long)virt_to_phys(x));
}

#define page_to_virt(page)	phys_to_virt(PFN_PHYS(page_to_pfn(page)))
#define virt_to_page(kaddr)	pfn_to_page(virt_to_pfn(kaddr))

#define __va(x)			((void *)phys_to_virt((phys_addr_t)(x)))
#define __pa(x)			virt_to_phys((void *)(unsigned long)(x))

static inline pte_t *pmd_page_vaddr(pmd_t pmd)
{
	return __va(pmd_val(pmd) & PHYS_MASK & (unsigned long)PAGE_MASK);
}

#define pte_none(pte)			(!pte_val(pte))

#define pte_clear(mm, addr, ptep)	set_pte_ext(ptep, __pte(0), 0)

#define pte_index(addr)			(((addr) >> PAGE_SHIFT) & \
							(PTRS_PER_PTE - 1))
#define pte_offset_kernel(pmd,addr)	(pmd_page_vaddr(*(pmd)) + \
							pte_index(addr))

static inline pte_t ptep_get_and_clear(struct mm_struct *mm,
			unsigned long address, pte_t *ptep)
{
	pte_t pte = *ptep;
	if (pte_val(pte))
		*ptep = __pte(0);
	return pte;
}

static inline pud_t *pud_alloc(struct mm_struct *mm, pgd_t *pgd,
				unsigned long address)
{
	return pud_offset(pgd, address);
}

static inline pmd_t *pmd_alloc(struct mm_struct *mm, pud_t *pud,
		unsigned long address)
{
	return pmd_offset(pud, address);
}

extern int __pte_alloc_kernel(pmd_t *pmd);
extern unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order);

#define pte_alloc_kernel(pmd, address)			\
	((unlikely(pmd_none(*(pmd))) && __pte_alloc_kernel(pmd)) ? \
		NULL : pte_offset_kernel(pmd, address))

#define __get_free_page(gfp_mask) \
			__get_free_pages((gfp_mask), 0)

#define PGALLOC_GFP	(GFP_KERNEL | __GFP_ZERO)

static inline void clean_pte_table(pte_t *pte)
{
	if (pte)
		memset(pte, 0, PAGE_SIZE);
}

/*
 * Allocate one PTE table.
 *
 * This actually allocates two hardware PTE tables, but we wrap this up
 * into one table thus:
 *
 *  +------------+
 *  | Linux pt 0 |
 *  +------------+
 *  | Linux pt 1 |
 *  +------------+
 *  |  h/w pt 0  |
 *  +------------+
 *  |  h/w pt 1  |
 *  +------------+
 */
static inline pte_t *pte_alloc_one_kernel(struct mm_struct *mm)
{
	pte_t *pte;

	pte = (pte_t *)__get_free_page(PGALLOC_GFP);
	if (pte)
		clean_pte_table(pte);

	return pte;
}

static inline void __pmd_populate(pmd_t *pmdp, phys_addr_t pte,
							pmdval_t prot)
{
	pmdval_t pmdval = (pte + PTE_HWTABLE_OFF) | prot;
	pmdp[0] = __pmd(pmdval);
}

static inline void pmd_populate_kernel(struct mm_struct *mm, pmd_t *pmdp,
							pte_t *ptep)
{
	/*
	 * The pmd must be loaded with the physical address of the PTE table
	 */
	__pmd_populate(pmdp, __pa(ptep), _PAGE_KERNEL_TABLE);
}

#define pfn_pte(pfn,prot)	__pte(PFN_PHYS(pfn) | pgprot_val(prot))
#define mk_pte(page,prot)	pfn_pte(page_to_pfn(page), prot)

static inline void set_pte_at(struct mm_struct *mm, unsigned long addr,
			pte_t *ptep, pte_t pteval)
{
	/* cpu_ca9mp_set_pte_ext */
	unsigned long ext = 0;

	/* Emulate hardware setup pte */
	*ptep = pteval;
}

static inline void *lowmem_page_address(const struct page *page)
{
	return page_to_virt(page);
}

static inline struct page *compound_head(struct page *page)
{
	unsigned long head = page->compound_head;

	if (head & 1)
		return (struct page *)(head - 1);
	return page;
}

static inline struct page *virt_to_head_page(const void *x)
{
	struct page *page = virt_to_page(x);

	return compound_head(page); 
}

#define page_address(page)	lowmem_page_address(page)

#define min_t(type, x, y)	({		\
		type __min1 = (x);		\
		type __min2 = (y);		\
		__min1 < __min2 ? __min1 : __min2; })

#define max_t(type, x, y)	({		\
		type __max1 = (x);		\
		type __max2 = (y);		\
		__max1 > __max2 ? __max1 : __max2; })

#define max(x, y)		({ x > y ? x : y; })
#define min(x, y)		({ x < y ? x : y; })
#define clamp_t(type, val, lo, hi)	min_t(type, max_t(type, val, lo), hi)

static inline unsigned long __ffs(unsigned long word)
{
	return __builtin_ctzl(word);
}

#define page_private(page)		((page)->private)
#define set_page_private(page, v)		((page)->private = (v))

static inline unsigned int page_order(struct page *page)
{
	/* PageBuddy() Must be checked by the caller */
	return page_private(page);
}

/*
 * For pages that are never mapped to userspace (and aren't PageSlab),
 * page_type may by used. Because it is initialised to -1, we invert the
 * sense of the bit, so __SetPageFoo *clears* the bit used for PageFoo,
 * and __ClearPageFoo *sets* the bit used for PageFoo. We reserve a few
 * high and low bits so that and underflow or overflow of page_mapcount()
 * won't be mistaken for a page type value.
 */

#define PAGE_TYPE_BASE		0xf0000000
/* Reserve	0x0000007f to catch underflows of page_mapcount */
#define PG_buddy		0x00000080

#define PageType(page, flag)				\
	((page->page_type & (PAGE_TYPE_BASE | flag)) == PAGE_TYPE_BASE)

/* PageBuddy() indicates that the page is free and in the buddy system
 * (see mm/page_alloc.c).
 */
static inline int PageBuddy(struct page *page)
{
	return PageType(page, PG_buddy);
}
static inline void __SetPageBuddy(struct page *page)
{
	page->page_type &= ~PG_buddy;
}
static inline void __ClearPageBuddy(struct page *page)
{
	page->page_type |= PG_buddy;
}

enum pageflags {
	PG_slab,
	PG_head,
	__NR_PAGEFLAGS,
};

static inline int PageSlab(struct page *page)
{
	return test_bit(PG_slab, &page->flags);
}

static inline void __SetPageSlab(struct page *page)
{
	__set_bit(PG_slab, &page->flags);
}

static inline void __ClearPageSlab(struct page *page)
{
	__clear_bit(PG_slab, &page->flags);
}

static inline int PageHead(struct page *page)
{
	return test_bit(PG_head, &page->flags);
}

static inline void __SetPageHead(struct page *page)
{
	__set_bit(PG_head, &page->flags);
}

static inline void __ClearPageHead(struct page *page)
{
	__clear_bit(PG_head, &page->flags);
}

static inline int PageTail(struct page *page)
{
	return page->compound_head & 1;
}

static inline int PageCompound(struct page *page)
{
	return test_bit(PG_head, &page->flags) || PageTail(page);
}

static inline void set_page_order(struct page *page, unsigned int order)
{
	set_page_private(page, order);
	__SetPageBuddy(page);
}

static inline void rmv_page_order(struct page *page)
{
	__ClearPageBuddy(page);
	set_page_private(page, 0);
}

extern struct zone BiscuitOS_zone;
static inline struct zone *page_zone(const struct page *page)
{
	return &BiscuitOS_zone;
}

static inline void set_page_count(struct page *page, int v)
{
	page->_refcount = v;
}

/*
 * Compound pages have a destructor function. Provide a
 * prototype for that function and accessor functions.
 * These are _only_ valid on the head of a compound page.
 */
typedef void compound_page_dtor(struct page *);

/* Keep the enum in sync with compound_page_dtors array in mm/page_alloc.c */
enum compound_dtor_id {
        NULL_COMPOUND_DTOR,
        COMPOUND_PAGE_DTOR,
        NR_COMPOUND_DTORS,
}; 

extern compound_page_dtor * const compound_page_dtors[];

static inline void set_compound_page_dtor(struct page *page,
		enum compound_dtor_id compound_dtor)
{
	page[1].compound_dtor = compound_dtor;
}

static inline compound_page_dtor *get_compound_page_dtor(struct page *page)
{
	return compound_page_dtors[page[1].compound_dtor];
}

static inline unsigned int compound_order(struct page *page)
{
	if (!PageHead(page))
		return 0;
	return page[1].compound_order;
}

static inline void set_compound_order(struct page *page, unsigned int order)
{
	page[1].compound_order = order;
}

static inline void set_compound_head(struct page *page, struct page *head)
{
	page->compound_head = (unsigned long)head + 1;
}

static inline unsigned long *compound_mapcount_ptr(struct page *page)
{
	return &page[1].compound_mapcount;
}

extern unsigned long nr_pages;;
extern struct page *__alloc_pages(gfp_t gfp_mask, unsigned int order);
static inline unsigned long totalram_pages(void)
{
	return (unsigned long)nr_pages;
}

static inline struct page *__alloc_pages_node(int nid, gfp_t gfp_mask,
                                        unsigned int order)
{
	return __alloc_pages(gfp_mask, order);
}

static inline struct page *alloc_pages_node(int nid, gfp_t gfp_mask,
							unsigned int order)
{
	return __alloc_pages_node(0, gfp_mask, order);
}

extern void free_pages(unsigned long addr, unsigned int order);

#define alloc_pages(gfp_mask, order) \
				alloc_pages_node(0, gfp_mask, order)
#define alloc_page(gfp_mask)	alloc_pages(gfp_mask, 0)
#define free_page(addr)		free_pages((addr), 0)

/* Free one PTE table */
static inline void pte_free_kernel(struct mm_struct *mm, pte_t *pte)
{
	if (pte)
		free_page((unsigned long)pte);
}

extern int memory_init(void);
extern void memory_exit(void);
/* Huge page sizes are variable */
extern unsigned int pageblock_order;
extern void __create_page_table(void);
extern unsigned long *mmu_vaddr_to_addr(unsigned long vaddr);
extern void __free_pages(struct page *page, unsigned int order);
static void prep_new_page(struct page *page, unsigned int order,
							gfp_t gfp_flags);
#endif
