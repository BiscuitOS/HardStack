/*
 * High-Memory
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

/* alloc page from Normal/Highmem */
static int instance_alloc_page(void)
{
	struct page *low_page, *high_page;
	void *low_addr, *high_addr;

	/* Alloc page from Normal */
	low_page = __alloc_pages(GFP_KERNEL, 0);

	/* Alloc page from Highmem */
	high_page = __alloc_pages(GFP_KERNEL | __GFP_HIGHMEM, 0);

	/* PFN information */
	printk("LOW PAGE:  %#lx\n", page_to_pfn(low_page));
	printk("High PAGE: %#lx\n", page_to_pfn(high_page));

	/* Virtual address */
	low_addr = page_address(low_page);
	printk("Low page Vaddr: %#lx\n", (unsigned long)low_addr);
	high_addr = page_address(high_page);
	printk("High page Vaddr: %#lx\n", (unsigned long)high_addr);

	/* free */
	__free_pages(low_page, 0);
	__free_pages(high_page, 0);

	return 0;
}

int main()
{
	memory_init();

	/* Running instance */
	instance_alloc_page();

	memory_exit();
	return 0;
}
