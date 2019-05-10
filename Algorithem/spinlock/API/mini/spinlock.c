/*
 * spinlock
 *
 * (C) 2019.05.08 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* Memory access
 *
 *
 *      +----------+
 *      |          |
 *      | Register |                                         +--------+
 *      |          |                                         |        |
 *      +----------+                                         |        |
 *            A                                              |        |
 *            |                                              |        |
 * +-----+    |      +----------+        +----------+        |        |
 * |     |<---o      |          |        |          |        |        |
 * | CPU |<--------->| L1 Cache |<------>| L2 Cache |<------>| Memory |
 * |     |<---o      |          |        |          |        |        |
 * +-----+    |      +----------+        +----------+        |        |
 *            |                                              |        |
 *            o--------------------------------------------->|        |
 *                         volatile/atomic                   |        |
 *                                                           |        |
 *                                                           +--------+
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* spinlock */
#include <linux/spinlock.h>

static spinlock_t BiscuitOS_lock;

static __init int spinlock_demo_init(void)
{

	/* Initialize spinlock */
	spin_lock_init(&BiscuitOS_lock);

	/* lock */
	spin_lock(&BiscuitOS_lock);
	
	__asm__ volatile ("nop");

	/* unlock */
	spin_unlock(&BiscuitOS_lock);
	

	return 0;
}
device_initcall(spinlock_demo_init);
