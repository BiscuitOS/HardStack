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

/*
 * instance kmem_cache_zalloc() from kmem_cache
 */
static int instance_kmem_call_zalloc(void)
{
	struct kmem_cache *s = kmem_cache_zalloc(kmem_cache, GFP_NOWAIT);

	printk("S %#lx\n", s);

	return 0;
}

int main()
{
	unsigned long *p;

	memory_init();

	/* Initialize Slub Allocator */
	kmem_cache_init();

	memory_exit();
	return 0;
}
