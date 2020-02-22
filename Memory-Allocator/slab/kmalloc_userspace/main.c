/*
 * Kmalloc Memory Allocator
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

/* private structure */
struct bs_struct {
	unsigned int idx;
	char name[32];
};

/* kmalloc and kfree */
static int instance_kmalloc_free(void)
{
	struct bs_struct *bp;

	bp = (struct bs_struct *)kmalloc(sizeof(struct bs_struct), 
								GFP_KERNEL);
	if (!bp) {
		printk("Kmalloc failed, no free memory.\n");
		return -ENOMEM;
	}

	sprintf(bp->name, "BiscuitOS-%x", 0x9091);
	bp->idx = 0x1016;
	printk("%s idx %#x\n", bp->name, bp->idx);

	kfree(bp);
	return 0;
}

/* kmalloc and kfree loop */
static int instance_kmalloc_loop(void)
{
	struct bs_struct *bp;
	int idx = 10;

	while (idx--) {
		bp = (struct bs_struct *)kmalloc(sizeof(struct bs_struct),
								GFP_KERNEL);
		printk("Kmalloc Address %#lx\n", (unsigned long)bp);
		kfree(bp);
	}

	return 0;
}

/* kzalloc and kfree */
static int instance_kzalloc(void)
{
	struct bs_struct *bp;

	bp = (struct bs_struct *)kzalloc(sizeof(struct bs_struct),
								GFP_KERNEL);
	if (!bp) {
		printk("Kzalloc failed, no free memory.\n");
		return -ENOMEM;
	}
	/* stantif zero area */
	if (!bp->name[0])
		printk("Kzalloc IDX %d\n", bp->idx);

	kfree(bp);
	return 0;
}

int main()
{
	unsigned long *p;

	memory_init();

	/* Initialize Slub Allocator */
	kmem_cache_init();

	/* Running instance */
	instance_kmalloc_free();
	instance_kmalloc_loop();
	instance_kzalloc();

	memory_exit();
	return 0;
}
