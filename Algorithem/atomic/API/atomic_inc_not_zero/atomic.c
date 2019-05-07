/*
 * atomic
 *
 * (C) 2019.05.05 <buddy.zhang@aliyun.com>
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

static atomic_t BiscuitOS_counter = ATOMIC_INIT(0);

/* atomic_* */
static __init int atomic_demo_init(void)
{
	/* Legacy data */
	printk("[0]Atomic: %d\n", atomic_read(&BiscuitOS_counter));

	/* increment unless the number is zero */
	atomic_inc_not_zero(&BiscuitOS_counter);

	/* New data */
	printk("[1]Atomic: %d\n", atomic_read(&BiscuitOS_counter));

	/* atomic_t is not zero. */
	atomic_add(1, &BiscuitOS_counter);
	/* atomic_t not zero */
	atomic_inc_not_zero(&BiscuitOS_counter);
	printk("[2]Atomic: %d\n", atomic_read(&BiscuitOS_counter));

	return 0;
}
device_initcall(atomic_demo_init);
