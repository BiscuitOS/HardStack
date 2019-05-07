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

static atomic_t BiscuitOS_counter = ATOMIC_INIT(8);

/* atomic_* */
static __init int atomic_demo_init(void)
{
	/* Atomic value before data  */
	printk("[0]Atomic: oiginal-> %d\n", atomic_read(&BiscuitOS_counter));

	/* Atomic inc */
	atomic_inc(&BiscuitOS_counter);

	/* Atomic value after data  */
	printk("[1]Atomic: new->     %d\n", atomic_read(&BiscuitOS_counter));

	return 0;
}
device_initcall(atomic_demo_init);
