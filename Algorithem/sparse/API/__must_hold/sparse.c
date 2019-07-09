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

static inline void bs_operations(spinlock_t *lock)
	__must_hold(lock)
{
	spin_unlock(lock);
}

static __init int sparse_demo_init(void)
{
	spinlock_t lock;

	/* Initialize spinlock */
	spin_lock_init(&lock);

	/* lock */
	spin_lock(&lock);

	/* Must lock before call here */
	bs_operations(&lock);

	return 0;
}
device_initcall(sparse_demo_init);
