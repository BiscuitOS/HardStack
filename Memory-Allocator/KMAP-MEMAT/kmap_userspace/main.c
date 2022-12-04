/*
 * Kmap Memory Allocator
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

/* FIXMAP/KMAP
 *
 * 0
 * +------------------+-----------+--------------------+----+-------+---+--+
 * |                  |           |                    |    |       |   |  |
 * |                  | KMAP AREA |    Kernel Space    | .. | FIXED |   |  |
 * |                  |           |                    |    |       |   |  |
 * +------------------+-----------+--------------------+----+-------+---+--+
 *                    A           A                         A       A   A
 *                    |           |                         |       |   |
 * PKMAP_BASE---------o           |                         |       |   |
 * PAGE_OFFSET--------------------o                         |       |   |
 *                                                          |       |   |
 *                                        FIXADDR_START-----o       |   |
 *                                        FIXADDR_TOP---------------o   |
 *                                        FIXADDR_END-------------------o
 */


/* KMAP HighMem zone page */
static int instance_kmap(void)
{
	struct page *page;
	void *mpage, *vpage;

	/* alloc page from highmem */
	page = __alloc_pages(GFP_KERNEL | __GFP_HIGHMEM, 0);
	if (PageHighMem(page))
		printk("HighMem-PFN:  %#lx\n", page_to_pfn(page));

	/* kmap mapping page */
	mpage = kmap(page);
	printk("KMAP Address: %#lx\n", (unsigned long)mpage);

	/* R/W */
	vpage = kaddr_to_vaddr(mpage);
	sprintf(vpage, "BiscuitOs-%x", 0x93);
	printk("Output Info:  %s\n", (char *)vpage);

	kunmap(page);
	__free_pages(page, 0);
	return 0;
}

/* KMAP Normal zone page */
static int instance_kmap_normal(void)
{
	struct page *page;
	void *mpage;

	/* alloc page from normal */
	page = __alloc_pages(GFP_KERNEL, 0);
	
	/* Kmap mapping page */
	mpage = kmap(page);
	sprintf(mpage, "BiscuitOS-%x", 0x96);
	printk("Output Info:  %s\n", (char *)mpage);

	kunmap(page);
	__free_pages(page, 0);

	return 0;
}

/* KMAP: mult-pages from highmem */
static int instance_kmap_mult(void)
{
	struct page *pages[10];
	int index;

	/* alloc */
	for (index = 0; index < 10; index++)
		pages[index] = __alloc_pages(GFP_KERNEL | __GFP_HIGHMEM, 0);

	/* Kmap mapping page */
	for (index = 0; index < 10; index++) {
		void *mpage;
		mpage = kmap(pages[index]);
		printk("KMAP Address: %#lx\n", (unsigned long)mpage);
	}

	/* kunmap and free */
	for (index = 0; index < 10; index++) {
		kunmap(pages[index]);
		__free_pages(pages[index], 0);
	}

	return 0;
}

/* FIXMAP: kmap_atomic from high-memory */
static int instance_kmap_atomic(void)
{
	struct page *page;
	void *mpage, *vpage;

	/* alloc */
	page = __alloc_pages(GFP_KERNEL | __GFP_HIGHMEM, 0);

	/* Mapping */
	mpage = kmap_atomic(page);
	printk("FIXMAP-ADDR:  %#lx\n", (unsigned long)mpage);
	
	/* Emulate MMU */
	vpage = kaddr_to_vaddr(mpage);
	sprintf(vpage, "BiscuitOS-%x", 0x89);
	printk("KMAP-Atomic:  %s\n", (char *)vpage);

	kunmap_atomic(mpage);
	__free_pages(page, 0);	

	return 0;
}

/* FIXMAP: kmap_atomic mult from high_memory */
static int instance_kmap_atomic_mult(void)
{
	struct page *pages[10];
	void *mpages[10];
	int index;

	/* alloc */
	for (index = 0; index < 10; index++) {
		pages[index] = __alloc_pages(GFP_KERNEL | __GFP_HIGHMEM, 0);
		/* mapping */
		mpages[index] = kmap_atomic(pages[index]);
		printk("FIXMAP Addr:  %#lx\n", (unsigned long)mpages[index]);
	}

	/* free */
	for (index = 0; index < 10; index++) {
		kunmap_atomic(mpages[index]);
		__free_pages(pages[index], 0);
	}
	return 0;
}

int main()
{
	memory_init();

	/* Kmap */
	kmap_init();

	/* Running instance */
	instance_kmap_atomic();
	instance_kmap_atomic_mult();
	instance_kmap();
	instance_kmap_normal();
	instance_kmap_mult();

	memory_exit();
	return 0;
}
