/*
 * Vmalloc Allocator
 *
 * (C) 2020.02.12 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linux/vmalloc.h"
#include "linux/buddy.h"
#include "linux/slub.h"

static struct vmap_block_queue vmap_block_queue;
static struct vfree_deferred vfree_deferred;
static struct vm_struct *vmlist;

void vmalloc_init(void)
{
	struct vmap_area *va;
	struct vm_struct *tmp;
	int i;

	for_each_possible_cpu(i) {
		struct vmap_block_queue *vbq;
		struct vfree_deferred *p;

		vbq = &vmap_block_queue;
		INIT_LIST_HEAD(&vbq->free);
		p = &vfree_deferred;
		init_llist_head(&p->list);
	}

	/* Import existing vmlist entries. */
	for (tmp = vmlist; tmp; tmp = tmp->next) {
		;
	}
}
