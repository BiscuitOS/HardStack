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
	spinlock_t __private lock;
};

static __init int sparse_demo_init(void)
{
	struct node n0;

	/* initialize spinlock */
	spin_lock_irq(&ACCESS_PRIVATE(&n0, lock));

	/* unlock */
	spin_unlock_irq(&ACCESS_PRIVATE(&n0, lock));

	return 0;
}
device_initcall(sparse_demo_init);
