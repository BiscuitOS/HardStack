/*
 * PCP Memory Allocator
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

/* alloc page from PCP */
static int instance_alloc_pcp(void)
{
	struct page *page;
	void *mpage;

	/* alloc from PCP */
	page = __alloc_pages(GFP_KERNEL, 0);

	/* mapping */
	mpage = page_address(page);
	sprintf(mpage, "BiscuitOS-%x", 0x86);
	printk("Page-PFN: %#lx\n", page_to_pfn(page));
	printk("Information: %s\n", (char *)mpage);

	/* free */
	__free_pages(page, 0);

	return 0;
}

/* hot and cold page */
static int instance_hot_cold(void)
{
	struct page *pages[20];
	int index;

	/* alloc 10 pages from cold list */
	for (index = 0; index < 10; index++) {
		pages[index] = __alloc_pages(GFP_KERNEL, 0);
		printk("Cold Page-PFN[%d]:  %#lx\n", index,
						page_to_pfn(pages[index]));
	}

	/* free 4 page to hot list */
	for (index = 6; index < 10; index++) {
		printk("Free-2-Hot PFN[%d]: %#lx\n", index,
						page_to_pfn(pages[index]));
		__free_pages(pages[index], 0);
	}

	
	/* alloc 10 page from hot list */
	for (index = 10; index < 20; index++) {
		pages[index] = __alloc_pages(GFP_KERNEL, 0);
		printk("Hot Page-PFN[%d]:  %#lx\n", index,
						page_to_pfn(pages[index]));
	}

	/* finish and free */
	for (index = 10; index < 20; index++)
		__free_pages(pages[index], 0);
	for (index = 0; index < 6; index++)
		__free_pages(pages[index], 0);

	return 0;
}

int main()
{
	memory_init();

	/* Running instance */
	instance_alloc_pcp();
	instance_hot_cold();

	memory_exit();
	return 0;
}
