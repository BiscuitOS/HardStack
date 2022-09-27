/*
 * BiscuitOS Kernel BiscuitOS Code
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/zsmalloc.h>
#include <linux/errno.h>
#include <linux/gfp.h>

static struct zs_pool *BiscuitOS_pool;

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	unsigned long handle;
	char buffer[64];
	void *dst;
	int r;

	/* Create Pool */
	BiscuitOS_pool = zs_create_pool("BiscuitOS-zsmalloc-pool");
	if (!BiscuitOS_pool) {
		printk("ZS Pool create failed.\n");
		r = -ENOMEM;
		goto err_zs_pool;
	}

	/* alloc */
	handle = zs_malloc(BiscuitOS_pool, 64,
				__GFP_KSWAPD_RECLAIM | __GFP_NOWARN |
				__GFP_HIGHMEM | __GFP_MOVABLE);
	if (!handle) {
		printk("ZS alloc failed.\n");
		r = -ENOMEM;
		goto err_alloc;
	}

	/* Compact */
	dst = zs_map_object(BiscuitOS_pool, handle, ZS_MM_RW);
	sprintf(dst, "BiscuitOS-zsmalloc-data %s", "Broiler");
	zs_unmap_object(BiscuitOS_pool, handle);
	dst = NULL;

	zs_compact(BiscuitOS_pool);

	/* Decompact */
	dst = zs_map_object(BiscuitOS_pool, handle, ZS_MM_RW);
	strcpy(buffer, dst);
	zs_unmap_object(BiscuitOS_pool, handle);
	printk("Decompact: %s\n", buffer);

	/* free */
	zs_free(BiscuitOS_pool, handle);
	zs_destroy_pool(BiscuitOS_pool);
	BiscuitOS_pool = NULL;

	return 0;

err_alloc:
	zs_destroy_pool(BiscuitOS_pool);
err_zs_pool:
	return r;
}

device_initcall(BiscuitOS_init);
