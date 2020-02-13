/*
 * MEMBLOCK Memory Allocator
 *
 * (C) 2020.02.13 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "linux/biscuitos.h"
#include "linux/memblock.h"

struct bs_struct {
	char name[32];
};

void dup_memblock(void)
{
	struct memblock_region *rgn;

	for_each_memblock(memory, rgn)
		printk("Valid Memory Regions:  %#lx - %#lx\n", 
			(unsigned long)rgn->base, 
			(unsigned long)(rgn->base + rgn->size));
	for_each_memblock(reserved, rgn)
		printk("Reserve Memory Region: %#lx - %#lx\n",
			(unsigned long)rgn->base,
			(unsigned long)(rgn->base + rgn->size));
}

/* alloc virtual memory from MEMBLOCK */
static int instance_memblock(void)
{
	struct bs_struct *bp;

	/* alloc */
	bp = memblock_alloc(sizeof(struct bs_struct), sizeof(unsigned long));
	if (!bp) {
		printk("memblock_alloc() failed, no free memory.\n");
		return -ENOMEM;
	}

	sprintf(bp->name, "BiscuitOS-%x", 0x90);
	printk("%s\n", bp->name);

	/* MEMBLOCK information */
	dup_memblock();

	/* free */
	memblock_free(__pa(bp), sizeof(struct bs_struct));
	return 0;
}

/* loop alloc and free virtual memory from MEMBLOCK */
static int instance_memblock_loop(void)
{
	struct bs_struct *bp;
	int cnt = 10;

	/* alloc */
	while (cnt--) {
		bp = memblock_alloc(sizeof(struct bs_struct), 
						sizeof(unsigned long));
		printk("BP Physical Address: %#lx\n", (unsigned long)__pa(bp));
		memblock_free(__pa(bp), sizeof(struct bs_struct));
	}

	/* MEMBLOCK information */
	dup_memblock();
	return 0;
}

/* alloc and free physical memory from MEMBLOCK */
static int instance_memblock_phys(void)
{
	phys_addr_t bp;
	char *virt;

	/* alloc */
	bp = memblock_phys_alloc(sizeof(struct bs_struct), 
						sizeof(unsigned long));
	virt = __va(bp);
	sprintf(virt, "BiscuitOS-%s", "phys");
	printk("%s:\nPhys %#lx\nVirt %#lx\n", virt, (unsigned long)bp, 
							(unsigned long)virt);

	/* MEMBLOCK information */
	dup_memblock();

	/* free */
	memblock_free(bp, sizeof(struct bs_struct));
	return 0;
}

/* alloc from bottom or top */
static int instance_memblock_top2bottom(void)
{
	phys_addr_t bp[10];
	int idx;

	/* setup direction: top to bottom  */
	memblock_set_bottom_up(false);
	printk("Alloc from top to bottom\n");
	/* alloc top to bottom */
	for (idx = 0; idx < 5; idx++) {
		bp[idx] = memblock_phys_alloc(sizeof(struct bs_struct),
						   sizeof(unsigned long));
		printk("BP[%d] Physical Address: %#lx\n", idx,
						(unsigned long)bp[idx]);
	}
	/* setup direction: bottom to top  */
	memblock_set_bottom_up(true);
	printk("Alloc from bottom to top\n");
	/* alloc bottom to top */
	for (; idx < 10; idx++) {
		bp[idx] = memblock_phys_alloc(sizeof(struct bs_struct),
						   sizeof(unsigned long));
		printk("BP[%d] Physical Address: %#lx\n", idx,
						(unsigned long)bp[idx]);
	}
	/* MEMBLOCK information */
	dup_memblock();

	/* free */
	for (idx = 0; idx < 10; idx++)
		memblock_free(bp[idx], sizeof(struct bs_struct));

	/* setup default direction */
	memblock_set_bottom_up(false);
	return 0;
}

/* trigger memblock double array */
static int instance_memblock_trigger_double_array(void)
{
	phys_addr_t bp[INIT_MEMBLOCK_REGIONS * 2];
	unsigned long offset = 0;
	int idx;

	for (idx = 0; idx < INIT_MEMBLOCK_REGIONS; idx++) {
		bp[idx] = PHYS_OFFSET + 2 * idx * PAGE_SIZE;
		memblock_reserve(bp[idx], PAGE_SIZE);
	}
	dup_memblock();
	/* Trigger */
	printk("Trigger memblock_double_array()\n");
	bp[idx] = PHYS_OFFSET + 2 * idx * PAGE_SIZE;
	memblock_reserve(bp[idx], PAGE_SIZE);

	/* continue alloc */
	for (idx++; idx < INIT_MEMBLOCK_REGIONS + 10; idx++) {
		bp[idx] = PHYS_OFFSET + 2 * idx * PAGE_SIZE;
		memblock_reserve(bp[idx], PAGE_SIZE);
	}
	dup_memblock();

	printk("Free All.....\n");
	/* free */
	while (idx-- > 0)
		memblock_free(bp[idx], PAGE_SIZE);
	dup_memblock();
	return 0;
}

int main()
{
	memory_init();

	/* Running instance */
	instance_memblock();
	instance_memblock_loop();
	instance_memblock_phys();
	instance_memblock_top2bottom();
	instance_memblock_trigger_double_array();

	memory_exit();

	return 0;
}
