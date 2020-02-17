/*
 * Vmalloc Memory Allocator
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
#include "linux/slub.h"
#include "linux/vmalloc.h"

/* vmalloc usage */
static int instance_vmalloc(void)
{
	unsigned long *base;
	unsigned long *addr;
	unsigned long vaddr;

	/* alloc */
	base = vmalloc(4 * PAGE_SIZE);

	/* locate vaddr */
	vaddr = (unsigned long)base + 0x89;

	/* Soft-MMU */
	addr = mmu_vaddr_to_addr(vaddr);
	sprintf((void *)addr, "BiscuitOS-%x", 0x90);
	printk("Output: %s\n", (char *)addr);

	/* Information */
	printk("VMALLOC AREA: %#lx - %#lx\n", (unsigned long)base,
				(unsigned long)base + 4 * PAGE_SIZE);
	printk("Vaddr %#lx -- Phys Addr: %#lx\n", vaddr, __pa(addr));

	/* free */
	vfree(base);

	return 0;
}

/* mult vmalloc 
 *
 * +-----------+---------+------+----+------+----+------+-----+----+
 * |           |         |      |    |      |    |      |     |    |
 * | Userspace | Normall | HOLE | A0 | HOLE | A1 | Hole | ... |    |
 * |           |         |      |    |      |    |      |     |    |
 * +-----------+---------+------+----+------+----+------+-----+----+
 *                              A    | <--> |<-->|            A
 *                              |       A     A               |
 *              VMALLOC_START---o       |     |               |
 *                                      |     |               |
 *              GUARD_AREA--------------o     |               |
 *                                            |               |
 *              VMALLOC_AREA------------------o               |
 *                                                            |
 *              VMALLOC_END-----------------------------------o
 */
static int instance_mult_vmalloc()
{
	unsigned long *base[10];
	int idx = 10;

	/* alloc */
	while (--idx >= 0) {
		base[idx] = vmalloc(PAGE_SIZE);

		printk("VMALLOC AREA: %#lx - %#lx\n", 
				(unsigned long)base[idx],
				(unsigned long)base[idx] + PAGE_SIZE);
	}

	/* dup RB-tree */
	dup_RBTREE();

	idx = 10;
	/* free */
	while (--idx >= 0)
		vfree(base[idx]);
}

int main()
{
	memory_init();

	/* Initialize Slub Allocator */
	kmem_cache_init();

	/* Initialize Vmalloc Allocator */
	vmalloc_init();

	/* Running instance */
	instance_vmalloc();
	instance_mult_vmalloc();

	memory_exit();
	return 0;
}
