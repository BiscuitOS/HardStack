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

/*
 * atomic_add_* (ARMv7 Cotex-A9MP)
 *
 * static inline int atomic_fetch_add_unless(atomic_t *v, int a, int u)
 * {
 *         int oldval, newval;
 *         unsigned long tmp;
 *
 *         smp_mb();
 *         prefetchw(&v->counter);
 * 
 *         __asm__ __volatile__ ("@ atomic_add_unless\n"
 * "1:     ldrex   %0, [%4]\n"
 * "       teq     %0, %5\n"
 * "       beq     2f\n"
 * "       add     %1, %0, %6\n"
 * "       strex   %2, %1, [%4]\n"
 * "       teq     %2, #0\n"
 * "       bne     1b\n"
 * "2:"
 *         : "=&r" (oldval), "=&r" (newval), "=&r" (tmp), "+Qo" (v->counter)
 *         : "r" (&v->counter), "r" (u), "r" (a)
 *         : "cc");
 * 
 *         if (oldval != u)
 *                 smp_mb();
 * 
 *         return oldval;
 * }
 */

#include <linux/kernel.h>
#include <linux/init.h>

static atomic_t BiscuitOS_counter = ATOMIC_INIT(8);

/* atomic_* */
static __init int atomic_demo_init(void)
{
	int val;

	/* Atomic add: Old == original */
	val = atomic_fetch_add_unless(&BiscuitOS_counter, 1, 8);

	printk("[0]Atomic: oiginal-> %d new-> %d\n", val, 
			atomic_read(&BiscuitOS_counter));

	/* Atomic add: Old != original */
	val = atomic_fetch_add_unless(&BiscuitOS_counter, 1, 5);

	printk("[1]Atomic: original-> %d new-> %d\n", val, 
			atomic_read(&BiscuitOS_counter));

	return 0;
}
device_initcall(atomic_demo_init);
