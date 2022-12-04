/*
 * Buddy Allocator
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "linux/buddy.h"
#include "linux/slub.h"

/* nr_pages for memory */
unsigned long nr_pages;
/* Simulate memory */
unsigned char *memory;
/* mem_map array */
struct page *mem_map;
/* Huge page sizes are variable */
unsigned int pageblock_order = 10;
/* Emulate Normal Zone */
struct zone BiscuitOS_zone = { .zone_name = "Normal", };
/* Emulate Highmem Zone */
struct zone BiscuitOS_highmem_zone = { .zone_name = "HighMem", };
/* pfn */
unsigned long low_max_pfn;
unsigned long low_pfn;
unsigned long high_pfn;

/*
 * Locate the struct page for both the matching buddy in our
 * pair (buddy1) and the combined O(n+1) page they form (page).
 *
 * 1) Any buddy B1 will have an order O twin B2 which satisfies
 *    the following equation:
 *
 *    For example, if the starting buddy (buddy2) is #8 its order
 *    1 buddy is #10:
 *
 *      B2 = 0 ^ (1 << 1) = 0 ^ 2 = 0000B ^ 0010B = 0010B = 2
 *      B2 = 2 ^ (1 << 1) = 2 ^ 2 = 0010B ^ 0010B = 0000B = 0
 *      B2 = 4 ^ (1 << 1) = 4 ^ 2 = 0100B ^ 0010B = 0110B = 6
 *      B2 = 6 ^ (1 << 1) = 6 ^ 2 = 0110B ^ 0010B = 0100B = 4
 *      B2 = 8 ^ (1 << 1) = 8 ^ 2 = 1000B ^ 0010B = 1010B = A
 *      B2 = A ^ (1 << 1) = A ^ 2 = 1010B ^ 0010B = 1000B = 8
 *      B2 = C ^ (1 << 1) = C ^ 2 = 1100B ^ 0010B = 1110B = E
 *      B2 = E ^ (1 << 1) = E ^ 2 = 1110B ^ 0010B = 1100B = C
 *
 *    For Figure, order 1
 *
 *    0      2      4      6      8      A      C      E      10
 *    +------+------+------+------+------+------+------+------+
 *    |      |      |      |      |      |      |      |      |
 *    |  B0  |  B1  |  B2  |  B3  |  B4  |  B5  |  B6  |  B7  |  
 *    |      |      |      |      |      |      |      |      |
 *    +------+------+------+------+------+------+------+------+
 *    | <- Pairs -> | <- Paris -> | <- Paris -> | <- Paris -> |
 *
 * 2) And buddy B will have and O+1 parent P which satisfies
 *    the following equation:
 *
 *      P = B & ~(1 << O)
 *
 *    For example, if the starting buddy is #8 it order 1:
 *
 *      P = 0 & ~(1 << 1) = 0000B & 1101B = 0000B
 *      P = 2 & ~(1 << 1) = 0010B & 1101B = 0000B
 *      P = 4 & ~(1 << 1) = 0100B & 1101B = 0100B
 *      P = 6 & ~(1 << 1) = 0110B & 1101B = 0100B
 *      P = 8 & ~(1 << 1) = 1000B & 1101B = 1000B
 *      P = A & ~(1 << 1) = 1010B & 1101B = 1000B
 *      P = C & ~(1 << 1) = 1100B & 1101B = 1100B
 *      P = E & ~(1 << 1) = 1110B & 1101B = 1100B
 *
 *    For Figure, order 1
 *
 *    0      2      4      6      8      A      C      E      10
 *    +------+------+------+------+------+------+------+------+
 *    |      |      |      |      |      |      |      |      |
 *    |  B0  |  B1  |  B2  |  B3  |  B4  |  B5  |  B6  |  B7  |  
 *    |      |      |      |      |      |      |      |      |
 *    +------+------+------+------+------+------+------+------+
 *    | <- Pairs -> | <- Paris -> | <- Paris -> | <- Paris -> |
 *    P0            P1            P2            P3
 *
 * Assumption: *_mem_map is contiguous at least up to MAX_ORDER.
 */
static inline unsigned long
__find_buddy_pfn(unsigned long page_pfn, unsigned int order)
{
	return page_pfn ^ (1 << order);
}

/*
 * This function checks whether a page is free && is the buddy
 * we can coalesce a page and its buddy if
 * (a) the buddy is not in a hole (check before calling!) &&
 * (b) the buddy is in the buddy system &&
 * (c) a page and its buddy have the same order &&
 * (d) a page and its buddy are in the same zone.
 *
 * For recording whether a page is in the system, we set PageBuddy.
 * Setting, clearing, and testing PageBuddy is serialized by 
 * zone->lock.
 *
 * For recording page's order, we use page_private(page);
 */
static inline int page_is_buddy(struct page *page, struct page *buddy,
					unsigned int order)
{
	if (PageBuddy(buddy) && page_order(buddy) == order) {
		if (page_zone(page) != page_zone(buddy))
			return 0;
		return 1;
	}
}

/*
 * Freeing functing for a buddy system allocator.
 *
 * The concept of a buddy system is to maintain direct-mapped table
 * (containing bit values) for memory blocks of various "orders".
 * The bottom level table contains the map for the smallest allocatable
 * units of memory (here, pages), and each level above it describes
 * pairs of units from the levels below, hence, "buddies".
 * At a high level, all that happens here is marking the table entry
 * at the bottom level available, and propagating the changes upward
 * as necessary, plus some accounting needed to play nicely with other
 * parts of the VM system.
 * At each level, we keep a list of pages, which are heads of continuous
 * free pages of length of (1 << order) and marked with PageByddy.
 * Page's order is recorded or freeing one, we can derive the state of
 * the other. That is, if we allocate a small block, and both were
 * free, the remainder of the region must be split into blocks. If
 * a block is freed, and its buddy is also free, then this triggers
 * coalescing into a block of larger size.
 *
 * -- nyc
 */

static inline void __free_one_page(struct zone *zone, struct page *page, 
			unsigned long pfn, unsigned int order)
{
	unsigned int max_order;
	unsigned long buddy_pfn;
	unsigned long combined_pfn;
	struct page *buddy;

	max_order = min_t(unsigned int, MAX_ORDER, pageblock_order + 1);

continue_merging:
	while (order < max_order - 1) {
		buddy_pfn = __find_buddy_pfn(pfn, order);	
		buddy = page + (buddy_pfn - pfn);

		if (!pfn_valid_within(buddy_pfn))
			goto done_merging;
		if (!page_is_buddy(page, buddy, order))
			goto done_merging;

		/*
		 * Our buddy is free and meger with it and move up
		 * one order.
		 */
		list_del(&buddy->lru);
		zone->free_area[order].nr_free--;
		rmv_page_order(buddy);
		combined_pfn = buddy_pfn & pfn;
		page = page + (combined_pfn - pfn);
		pfn = combined_pfn;
		order++;
	}

done_merging:
	set_page_order(page, order);

	/* We have no PCP, so only goto pcp_emulate */
	if (order == 0)
		goto pcp_emulate;

	/*
	 * If this is not the largest possible page, check if the buddy
	 * of the next-highest order is free. If it is, it's possible
	 * that pages are being free that will coalesce soon. In case,
	 * that is happening, add the free page to the tail of the list
	 * so it's less likely to be used soon and more likely to be meged
	 * as a higher order page.
	 */
	if ((order < MAX_ORDER-2) && pfn_valid_within(buddy_pfn)) {
		struct page *higher_page, *higher_buddy;

		combined_pfn = buddy_pfn & pfn;
		higher_page = page + (combined_pfn - pfn);
		buddy_pfn = __find_buddy_pfn(combined_pfn, order + 1);
		higher_buddy = higher_page + (buddy_pfn - combined_pfn);
		if (pfn_valid_within(buddy_pfn) &&
		    page_is_buddy(higher_page, higher_buddy, order + 1)) {
			list_add_tail(&page->lru,
				&zone->free_area[order].free_list[0]);
			goto out;
		}
		goto pcp_emulate;
	}

pcp_emulate:
	list_add(&page->lru, &zone->free_area[order].free_list[0]);
out:
	zone->free_area[order].nr_free++;
}

static void __free_pages_ok(struct page *page, unsigned int order)
{
	unsigned long pfn = page_to_pfn(page);

	__free_one_page(page_zone(page), page, pfn, order);
}

static inline void free_the_page(struct page *page, unsigned int order)
{
	__free_pages_ok(page, order);
}

void __free_pages(struct page *page, unsigned int order)
{
	free_the_page(page, order);
}

/*
 * The order of subdivision here is critical for the IO subsystem.
 * Please do not alter this order without good reasons and regression
 * testing. Specifically, as large blocks of memory are subdivided,
 * the order in which smaller blocks are delivered depends on the
 * order they're subdivided in this function. This is the primary
 * factor influencing the order in which pages are delivered to the
 * IO subsystem according to empirical testing, and this is also
 * justified by considering the behavior of a buddy system containing
 * a single large block of memory acted on by a series of small
 * allocations. This behavior is a critical factor in sglist merging's
 * success.
 */
static inline void expand(struct zone *zone, struct page *page,
			int low, int high, struct free_area *area)
{
	unsigned long size = 1 << high;

	while (high > low) {
		area--;
		high--;
		size >>= 1;

		list_add(&page[size].lru, &area->free_list[0]);
		area->nr_free++;
		set_page_order(&page[size], high);
	}
}

/*
 * Go through the free lists for the given migratetype and remove
 * the smallest available page from the freelist.
 */
static inline
struct page *__rmqueue_smallest(struct zone *zone, unsigned int order)
{
	unsigned int current_order;
	struct free_area *area;
	struct page *page;

	/* Find a page for appropriate size in the preferred list */
	for (current_order = order; current_order < MAX_ORDER; 
							++current_order) {
		area = &(zone->free_area[current_order]);
		page = list_first_entry_or_null(&area->free_list[0],
							struct page, lru);
		if (!page)
			continue;
		list_del(&page->lru);
		rmv_page_order(page);
		area->nr_free--;
		expand(zone, page, order, current_order, area);
		return page;
	}
	return NULL;
}

/*
 * Allocate a page from the given zone. Use pcplists for order-0
 * allocations.
 */
static inline
struct page *rmqueue(struct zone *zone, unsigned int order,
						gfp_t gfp_flags)
{
	struct page *page;

	if (likely(order == 0)) {
		/* via PCP */;
	}

	/* We most definitely don't want callers attempting to
	 * allocate greater than order-1 page units with __GFP_NOFAIL.
	 */
	page = __rmqueue_smallest(zone, order);
	return page;
}

/*
 * get_page_from_freelist goes through the zonelist trying to
 * allocate a page.
 */
static struct page *
get_page_from_freelist(gfp_t gfp_mask, unsigned int order)
{
	struct zone *zone = &BiscuitOS_zone;
	struct page *page;

	if (gfp_mask & __GFP_HIGHMEM)
		zone = &BiscuitOS_highmem_zone;

	page = rmqueue(zone, order, gfp_mask);
	if (page) {
		prep_new_page(page, order, gfp_mask);
	}
	
	return page;
}

/*
 * This is the 'heart' of the zoned buddy allocator
 */
struct page *__alloc_pages(gfp_t gfp_mask, unsigned int order)
{
	struct page *page;

	/* First allocation attempt */
	page = get_page_from_freelist(gfp_mask, order);
	return page;	
}

/*
 * PHYS_OFFSET                                         
 * | <----------------- MEMORY_SIZE ------------------> | <- HighMem ->|
 * +---------------+------------------------------------+--------------+
 * |               |                                    |              |
 * |               |                                    |              |
 * |               |                                    |              |
 * +---------------+------------------------------------+--------------+
 * | <- mem_map -> |
 *
 */
int memory_init(void)
{
	unsigned long start_pfn, end_pfn;
	struct zone *zone;
	int order, index;

	/* Emulate Memory Region */
	memory = (unsigned char *)malloc(MEMORY_SIZE + HIGHMEM_SIZE);

	/* Establish mem_map[] */
	mem_map = (struct page *)(unsigned long)memory;

	/* Initialize all pages */
	nr_pages = (MEMORY_SIZE + HIGHMEM_SIZE) / PAGE_SIZE;
	for (index = 0; index < nr_pages; index++) {
		struct page *page = &mem_map[index];

		INIT_LIST_HEAD(&page->lru);
		page->page_type |= PAGE_TYPE_BASE;
	}

	/* Initialize Zone */
	for (order = 0; order < MAX_ORDER; order++) {
		/* Normal zone */
		zone = &BiscuitOS_zone;
		INIT_LIST_HEAD(&zone->free_area[order].free_list[0]);
		zone->free_area[order].nr_free = 0;
		/* HighMem zone */
		zone = &BiscuitOS_highmem_zone;
		INIT_LIST_HEAD(&zone->free_area[order].free_list[0]);
		zone->free_area[order].nr_free = 0;
	}

	/* free all page into Buddy Allocator */
	start_pfn = PFN_UP(PHYS_OFFSET);
	end_pfn = PFN_DOWN(PHYS_OFFSET + MEMORY_SIZE);
	/* defind zone high and normal */
	low_pfn = start_pfn;
	low_max_pfn = end_pfn;
	high_pfn = end_pfn;
	end_pfn = PFN_DOWN(PHYS_OFFSET + MEMORY_SIZE + HIGHMEM_SIZE);

	while (start_pfn < end_pfn) {
		int order = min_t(unsigned int,
					MAX_ORDER - 1UL, __ffs(start_pfn));

		while (start_pfn + (1UL << order) > end_pfn)
			order--;

		/* Free page into Buddy Allocator */
		__free_pages(pfn_to_page(start_pfn), order);

		start_pfn += (1UL << order);
	}

	printk("BiscuitOS Kmap Memory Allocator\n");
	printf("Real Physical Memory:  %#lx - %#lx\n", 
		(unsigned long)PHYS_OFFSET, 
		(unsigned long)(PHYS_OFFSET + MEMORY_SIZE + HIGHMEM_SIZE));
	printk("Normal Physical Areas: %#lx - %#lx\n",
			PFN_PHYS(low_pfn), PFN_PHYS(low_max_pfn));
	printk("HighMem Physical Area: %#lx - %#lx\n",
			PFN_PHYS(low_max_pfn), PFN_PHYS(end_pfn));
	printk("Virtual Memory:        %#lx - %#lx\n", 
					(unsigned long)memory,
					(unsigned long)memory + MEMORY_SIZE);
	printf("mem_map[] contains %#lx pages, page size %#lx\n", nr_pages,
						(unsigned long)PAGE_SIZE);

	page_address_init();
	/* Kmem cache init */
	kmem_cache_init();
	/* Establish page-table-directory */
	__create_page_table();
	return 0;
}

void prep_compound_page(struct page *page, unsigned int order)
{
	int i;
	int nr_pages = 1 << order;

	set_compound_page_dtor(page, COMPOUND_PAGE_DTOR);
	set_compound_order(page, order);
	__SetPageHead(page);
	for (i = 1; i < nr_pages; i++) {
		struct page *p = page + i;

		set_page_count(p, 0);
		set_compound_head(p, page);
	}
	*(unsigned long *)(compound_mapcount_ptr(page)) = -1;
}

static void prep_new_page(struct page *page, unsigned int order,
							gfp_t gfp_flags)
{
	int i;

	if (gfp_flags & __GFP_ZERO)
		printk("NEED clear %s\n", __func__);

	if (order && (gfp_flags & __GFP_COMP))
		prep_compound_page(page, order);
}

void free_compound_page(struct page *page)
{
	printk("NEED... %s\n", __func__);
}

compound_page_dtor * const compound_page_dtors[] = {
	NULL,
	free_compound_page,
};

void memory_exit(void)
{
	free(memory);
}
