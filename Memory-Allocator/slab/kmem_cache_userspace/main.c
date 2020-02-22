/*
 * Kmem_cache Memory Allocator
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

int main()
{
	unsigned long *p;

	memory_init();

	/* Initialize Slub Allocator */
	kmem_cache_init();

	/* Running instance */
	instance_kmem_cache_create();
	instance_kmem_cache_loop();

	memory_exit();
	return 0;
}
