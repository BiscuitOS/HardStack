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

static __init int sparse_demo_init(void)
{
	spinlock_t lock;
	int cond = 1;

	/* Initialize spinlock */
	spin_lock_init(&lock);

	/* lock */
	(void)__cond_lock(lock, cond);

	if (cond) {
		/* need unlock */
		__release(lock);
	}

	return 0;
}
device_initcall(sparse_demo_init);
