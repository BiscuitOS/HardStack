/*
 * mount
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include "linux/bsfs.h"

struct kmem_cache *bsfs_node_cache;

void bsfs_init(void)
{
	bsfs_node_cache = kmem_cache_create("bsfs_node_cache",
				sizeof(struct bsfs_node),
				0,
				SLAB_PANIC | SLAB_TYPESAFE_BY_RCU,
				NULL);
}
