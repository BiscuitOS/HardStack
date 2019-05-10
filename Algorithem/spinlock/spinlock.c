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

	unsigned long tmp;
	u32 newval;
	arch_spinlock_t lockval;
	arch_spinlock_t *lock = (arch_spinlock_t *)&BiscuitOS_lock;

	__asm__ volatile (
"1:	ldrex	%0, [%3]\n\t"
"	add	%1, %0, %4\n\t"
"	strex	%2, %1, [%3]"
	: "=&r" (lockval), "=&r" (newval), "=&r" (tmp)
	: "r" (&lock->slock), "I" (1 << TICKET_SHIFT)
	: "cc");

	/* Execute success */
	if (!tmp) {
		printk("Lock value: %#x\n", lockval.slock);
		printk("new value:  %#x\n", newval);
		printk("Owner:      %#x\n", lockval.tickets.owner);
		printk("next:       %#x\n", lockval.tickets.next);
	}

	return 0;
}
device_initcall(spinlock_demo_init);
