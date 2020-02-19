/*
 * Buddy Allocator (contains Highmem)
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

#include "linux/buddy.h"

/*
 * Instance alloc 16-Pages and then free it.
 *
 * For example, buddy allocator contain 4-page-block which 
 * contains (1<<a) pages. Other null on free list.
 *
 * 1) Before
 *    0    1    2    3    4    5    6    7    8    9    a
 *    +----+----+----+----+----+----+----+----+----+----+
 *                                                     (A)
 *                                                     (B)
 *                                                     (C)
 *                                                     (D)
 *
 *    PFN: A = 0x60c00
 *
 * 2) Allocate
 *    Alloc 16 pages from free list, but order-4 contains null pages,
 *    so buddy will alloc from high order and expand buddy-page on
 *    special free list.
 * 
 *
 *    0    1    2    3    4    5    6    7    8    9    a
 *    +----+----+----+----+----+----+----+----+----+----+
 *                       (A0) (A1) (A2) (A3) (A4) (A5) (B)
 *                                                     (C)
 *                                                     (D)
 *
 *    PFN: A0 = 0x60c10
 *         A1 = 0x60c20
 *         A2 = 0x60c40
 *         A3 = 0x60c80
 *         A4 = 0x60d00
 *         A5 = 0x60e00
 *         16-pages: 0x60c00
 *
 *    So, A = A0 + A1 + A2 + A3 + A4 + A5 + 16-pages
 *
 * 3) Free
 *    When free pages, buddy allocator will still find and merge buddy 
 *    page util no buddy page.
 *
 *    0    1    2    3    4    5    6    7    8    9    a
 *    +----+----+----+----+----+----+----+----+----+----+
 *                                                     (A)
 *                                                     (B)
 *                                                     (C)
 *                                                     (D)
 *
 *    PFN: A = 0x60c00
 */
static int instance_16_pages(void)
{
	struct page *page;
	int order = 4; /* 1 << 4 => 16 pages */

	/* alloc 16 pages */
	page = __alloc_pages(GFP_KERNEL | __GFP_HIGHMEM, order);
	if (!page) {
		printk("Error: can't alloc pages from buddy.\n");
		return -1;
	}

	printk("16-Pages Phys:  %#lx PFN: %#lx\n", 
				PFN_PHYS(page_to_pfn(page)), 
				page_to_pfn(page));

	/* Free 16 pages */
	__free_pages(page, order);
	return 0;
}

/*
 * Instance alloc 16/128-Pages and then free it.
 *
 * For example, buddy allocator contain 4-page-block which 
 * contains (1<<a) pages. Other null on free list.
 *
 * 1) Before
 *    0    1    2    3    4    5    6    7    8    9    a
 *    +----+----+----+----+----+----+----+----+----+----+
 *                                                     (A)
 *                                                     (B)
 *                                                     (C)
 *                                                     (D)
 *
 *    PFN: A = 0x60c00
 *
 * 2) Allocate
 *    Alloc 16 pages from free list, but order-4 contains null pages,
 *    so buddy will alloc from high order and expand buddy-page on
 *    special free list.
 * 
 *
 *    0    1    2    3    4    5    6    7    8    9    a
 *    +----+----+----+----+----+----+----+----+----+----+
 *                       (A0) (A1) (A2) (A3) (A4) (A5) (B)
 *                                                     (C)
 *                                                     (D)
 *
 *    PFN: A0 = 0x60c10
 *         A1 = 0x60c20
 *         A2 = 0x60c40
 *         A3 = 0x60c80
 *         A4 = 0x60d00
 *         A5 = 0x60e00
 *         16-pages: 0x60c00
 *
 *    So, A = A0 + A1 + A2 + A3 + A4 + A5 + 16-pages
 *
 *    Then alloc 128 pages from free list, now 7-order free list contains
 *    a valid page-block "A3", so buddy allocator will get page block from
 *    "A3", then free list as follow:
 *
 *
 *    0    1    2    3    4    5    6    7    8    9    a
 *    +----+----+----+----+----+----+----+----+----+----+
 *                       (A0) (A1) (A2)      (A4) (A5) (B)
 *                                                     (C)
 *                                                     (D)
 *
 *    PFN: A0 = 0x60c10
 *         A1 = 0x60c20
 *         A2 = 0x60c40
 *         A4 = 0x60d00
 *         A5 = 0x60e00
 *         128-pages: 0x60c80
 *
 * 3) Free
 *    When freeing, buddy allocator will merge 16-pageblock with
 *    buddy page-block. But "A3" has allocated by 128-pages, the 
 *    16-pages is not the largest possible page, check if the buddy
 *    of the next highest order is free. The higher order page is
 *    "A4" and it's free, it's possible that pages are being free 
 *    that will coalesce soon. In case, that is happening, add the
 *    16-pages to the tail of the list so it's less likely to be
 *    used soon and more likely to be merged as a higher orddder page.
 *    as follow figure:
 *
 *
 *    0    1    2    3    4    5    6    7    8    9    a
 *    +----+----+----+----+----+----+----+----+----+----+
 *                                (A2B)      (A4) (A5) (B)
 *                                                     (C)
 *                                                     (D)
 *
 *    "A2B" is combined by A2, and it's less likely to be used soon
 *    and more likely to be merged as a higher order page.
 *    Then free 128-pages, it's buddy page-block is "A2B" and it's
 *    free. so buddy allocator merge these page continue. as figure:
 *
 *    0    1    2    3    4    5    6    7    8    9    a
 *    +----+----+----+----+----+----+----+----+----+----+
 *                                                     (A)
 *                                                     (B)
 *                                                     (C)
 *                                                     (D)
 *
 */
static int instance_16_128_pages(void)
{
	struct page *page16, *page128;

	/* alloc 16 pages */
	page16 = __alloc_pages(GFP_KERNEL | __GFP_HIGHMEM, 4);
	if (!page16) {
		printk("Error: can't alloc 16 pages from buddy.\n");
		return -1;
	}

	/* alloc 128 pages */
	page128 = __alloc_pages(GFP_KERNEL | __GFP_HIGHMEM, 7);
	if (!page128) {
		printk("Error: can't alloc 128 pages from buddy.\n");
		__free_pages(page16, 4);
		return -1;
	}

	printk("16-Pages Phys:  %#lx PFN: %#lx\n", 
				PFN_PHYS(page_to_pfn(page16)),
				page_to_pfn(page16));
	printk("128-Pages Phys: %#lx PFN: %#lx\n", 
				PFN_PHYS(page_to_pfn(page128)),
				page_to_pfn(page128));

	/* free 16-pages */
	printk("Freeing 16-pages.\n");
	__free_pages(page16, 4);
	printk("Freeing 128-pages.\n");
	__free_pages(page128, 7);
	return 0;
}

/* page usage */
static int instance_page_address(void)
{
	struct page *page;
	void *vpage;

	/* alloc */
	page = __alloc_pages(GFP_KERNEL | __GFP_HIGHMEM, 0);

	/* address */
	vpage = page_address(page);
	printk("Page-PFN: %#lx Address %#lx\n", page_to_pfn(page),
			(unsigned long)page_address(page));

	if (vpage) {
		/* use */
		sprintf(vpage, "BiscuitOs-%x", 0x88);
		printk("Buddy Output: %s\n", (char *)vpage);
	} else
		printk("Page no-mapping.\n");

	/* free */
	__free_pages(page, 0);
}

int main()
{
	memory_init();

	/* Running instance */
	instance_16_pages();
	instance_16_128_pages();
	instance_page_address();

	memory_exit();
	return 0;
}
