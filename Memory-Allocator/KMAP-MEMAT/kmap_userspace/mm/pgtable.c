/*
 * Page-Table
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
#include <malloc.h>

#include "linux/pgtable.h"
#include "linux/buddy.h"
#include "linux/highmem.h"
#include "linux/slub.h"

struct mm_struct init_mm;
/* page-table-directory */
pgd_t *swapper_pg_dir;

/* Establish page table on boot stage */
void __create_page_table(void)
{
	swapper_pg_dir = (pgd_t *)kmalloc(PG_DIR_SIZE, GFP_KERNEL);
	/* Clear page table */
	memset(swapper_pg_dir, 0, PG_DIR_SIZE);
	init_mm.pgd = swapper_pg_dir;

	printk("PAGE-Table-Directory: %#lx - %#lx\n", 
			(unsigned long)swapper_pg_dir,
			(unsigned long)swapper_pg_dir + PG_DIR_SIZE);
        printk("KMAP Page-Direct:     %#lx - %#lx\n", 
			(unsigned long)pgd_offset_k(LAST_PKMAP),
			(unsigned long)pgd_offset_k(PKMAP_BASE));
}

