/*
 * Slub Memory Allocator
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

/*
 * Create kmem_cache and alloc cache
 */
static int instance_kmem_cache_create(void)
{
	struct kmem_cache *s;
	struct bs_struct *array[10];
	int idx;

	/* Create kmem cache */
	s = kmem_cache_create("BiscuitOS",
					sizeof(struct bs_struct),
					0,
					SLAB_HWCACHE_ALIGN,
					NULL);
	if (IS_ERR(s)) {
		printk("ENOMEM For Create Cache BiscuitOS\n");
		return -ENOMEM;
	}
	printk("SLUB create kmem_cache: %s size %d\n", s->name, s->size);

	/* Allocate from cache */
	for (idx = 0; idx < 10; idx++) {
		/* Allocate a cache */
		array[idx] = kmem_cache_zalloc(s, GFP_NOWAIT);
		sprintf(array[idx]->name, "BiscuitOS-%d", idx);
		array[idx]->idx = idx;
		printk("%s -> %d\n", array[idx]->name, array[idx]->idx);
	}

	/* Free to cache */
	for (idx = 0; idx < 10; idx++)
		kmem_cache_free(s, array[idx]);
	
	/* Destroy kmem cache */
	kmem_cache_destroy(s);
	return 0;
}

/*
 * instance kmem_cache_alloc and kmem_cache_free
 */
static int instance_kmem_cache_loop(void)
{
	struct kmem_cache *s;
	struct bs_struct *bp, *bp1, *bp2;
	int cnt = 5;

	/* Cache create */
	s = kmem_cache_create("BiscutOS-loop",
				sizeof(struct bs_struct),
				0,
				SLAB_HWCACHE_ALIGN,
				NULL);


	/* alloc-free */
	while (cnt--) {
		bp = kmem_cache_zalloc(s, GFP_NOWAIT);
		printk("Alloc Address: %#lx\n", (unsigned long)bp);
		kmem_cache_free(s, bp);
	}

	cnt = 5;
	/* alloc-free */
	while (cnt--) {
		bp = kmem_cache_zalloc(s, GFP_NOWAIT);
		bp1 = kmem_cache_zalloc(s, GFP_NOWAIT);
		printk("Alloc Address 0: %#lx\n", (unsigned long)bp);
		printk("Alloc Address 1: %#lx\n", (unsigned long)bp1);
		kmem_cache_free(s, bp);
		bp2 = kmem_cache_zalloc(s, GFP_NOWAIT);
		printk("Alloc Address 2: %#lx\n", (unsigned long)bp2);
		kmem_cache_free(s, bp2);
		kmem_cache_free(s, bp1);
	}

	kmem_cache_destroy(s);
	return 0;
}

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

/* name allocate */
static int instance_name_alloc(void)
{
	char *tmp = "BiscuitOS";
	char *tmx = "BiscuitOS-SpaceX";
	const char *name_memory;
	const char *name_dup;

	name_memory = kstrdup_const(tmp, GFP_KERNEL);
	name_dup = kstrdup(tmx, GFP_KERNEL);

	printk("Kstrdup_const: %s\n", name_memory);
	printk("kstrdup:       %s\n", name_dup);

	kfree(name_dup);
	kfree_const(name_memory);
	return 0;
}

/* format name memory alloc */
static int instance_format_name_alloc(void)
{
	char *tmx = "BiscuitOS";
	int idx = 0x90;
	const char *name_format;
	
	name_format = kasprintf(GFP_KERNEL, "%s-%x", tmx, idx);
	printk("Format name:   %s\n", name_format);
	kfree(name_format);
	return 0;
}

int main()
{
	unsigned long *p;

	memory_init();

	/* Initialize Slub Allocator */
	kmem_cache_init();

	/* Running instance */
	instance_kmem_cache_create();
	instance_kmem_cache_loop();
	instance_kmalloc_free();
	instance_kmalloc_loop();
	instance_kzalloc();
	instance_name_alloc();
	instance_format_name_alloc();

	memory_exit();
	return 0;
}
