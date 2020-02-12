#ifndef _BISCUITOS_BUDDY_H
#define _BISCUITOS_BUDDY_H

#include "linux/list.h"
#include "linux/gfp.h"
#include "linux/biscuitos.h"
#include "linux/bitmap.h"

#define PAGE_SHIFT	12 /* 4KByte Page */
#define PAGE_SIZE	(1 << PAGE_SHIFT)
#define PAGE_MASK	(~((1 << PAGE_SHIFT) - 1))
#define PFN_ALIGN(x)	(((unsigned long)(x) + (PAGE_SIZE - 1)) & PAGE_MASK)
#define PFN_UP(x)	(((x) + PAGE_SIZE-1) >> PAGE_SHIFT)
#define PFN_DOWN(x)	((x) >> PAGE_SHIFT)
#define PFN_PHYS(x)	((phys_addr_t)(x) << PAGE_SHIFT)
#define PHYS_PFN(x)	((unsigned long)(x) >> PAGE_SHIFT)

/* Configuration Memory Region */
#define MEMORY_SIZE	CONFIG_MEMORY_SIZE
/* Configuration Basic Physical Address */
#define PHYS_OFFSET	CONFIG_PHYS_BASE

#define PFN_OFFSET	PHYS_PFN(PHYS_OFFSET)

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

#define max(x, y)		({ x > y ? x : y; })
#define min(x, y)		({ x < y ? x : y; })

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

extern int memory_init(void);
extern void memory_exit(void);
/* Huge page sizes are variable */
extern unsigned int pageblock_order;
extern void __free_pages(struct page *page, unsigned int order);
extern struct page *__alloc_pages(gfp_t gfp_mask, unsigned int order);

static void prep_new_page(struct page *page, unsigned int order,
							gfp_t gfp_flags);
#endif
