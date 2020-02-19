#ifndef _BISCUITOS_BUDDY_H
#define _BISCUITOS_BUDDY_H

#include "linux/list.h"

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
/* Configuration BATCH size */
#define BATCH_SIZE	CONFIG_BATCH_SIZE

#define PFN_OFFSET	PHYS_PFN(PHYS_OFFSET)

/* Buddy Allocator Order */
#define MAX_ORDER	11

/* GFP flag combinations */
#define GFP_KERNEL	0x10000000

#define printk(...)	printf(__VA_ARGS__)
#define max(x, y)	((x) > (y) ? (x) : (y))

typedef unsigned long phys_addr_t;
typedef unsigned long gfp_t;

struct free_area {
	struct list_head free_list[1];
	unsigned long nr_free;
};

struct per_cpu_pages {
	int count;	/* number of pages in the list */
	int high;	/* high watermark, emptying needed */
	int batch;	/* chunk size for buddy add/remove */

	/* List of pages */
	struct list_head lists[1];
};

struct zone {
	struct per_cpu_pages *pcp;
	/* free areas of different sizes */
	struct free_area free_area[MAX_ORDER];
	unsigned long managed_pages;
};

struct page {
	unsigned int page_type;
	unsigned long private;
	struct list_head lru;
};

/* PFN and PHYS */
#define pfn_to_page(pfn)	(mem_map + ((pfn) - PFN_OFFSET))
#define page_to_pfn(page)	((unsigned long)((page) - mem_map) + \
							PFN_OFFSET)
#define pfn_valid_within(pfn)	(1)

/* Physical and Virtual */
extern unsigned char *memory;
extern struct page *mem_map;
static inline phys_addr_t virt_to_phys(const volatile void *x)
{
	return ((unsigned long)(x) - (unsigned long)memory) + 
							PHYS_OFFSET;
}

static inline void *phys_to_virt(phys_addr_t x)
{
	return (void *)((x - PHYS_OFFSET) + (unsigned long)memory);
}

#define min_t(type, x, y)	({		\
		type __min1 = (x);		\
		type __min2 = (y);		\
		__min1 < __min2 ? __min1 : __min2; })

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

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

extern struct zone BiscuitOS_zone;
static inline struct zone *page_zone(const struct page *page)
{
	return &BiscuitOS_zone;
}

#define __va(x)			((void *)phys_to_virt((phys_addr_t)(x)))
#define page_to_virt(x)		__va(PFN_PHYS(page_to_pfn(x)))

static inline void *lowmem_page_address(const struct page *page)
{
	return page_to_virt(page);
}

static inline unsigned long zone_managed_pages(struct zone *zone)
{
	return zone->managed_pages;
}

extern struct page *mem_map;
extern int memory_init(void);
extern void memory_exit(void);
extern void *page_address(const struct page *page);
/* Huge page sizes are variable */
extern unsigned int pageblock_order;
extern void __free_pages(struct page *page, unsigned int order);
extern struct page *__alloc_pages(gfp_t gfp_mask, unsigned int order);
#endif
