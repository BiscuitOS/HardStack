/*
 * Sparse.
 *
 * (C) 2019.07.01 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* sparse macro */
#include <linux/types.h>
/* spinlock */
#include <linux/spinlock.h>

struct node {
	spinlock_t lock;
};

/* lock with __releases() and __acquires() */
static inline void bs_connect(struct node *np)
	__releases(np->lock)
	__acquires(np->lock)
{
	spin_lock(&np->lock);
}

/* unlock with __acquires() and  __releases() */
static inline void bs_disconnect(struct node *np)
	__releases(np->lock)
	__acquires(np->lock)
{
	spin_unlock(&np->lock);
}

static __init int sparse_demo_init(void)
{
	struct node n0;

	/* init spinlock */
	spin_lock_init(&n0.lock);

	/* lock */
	bs_connect(&n0);

	/* unlock */
	bs_disconnect(&n0);

	return 0;
}
device_initcall(sparse_demo_init);
