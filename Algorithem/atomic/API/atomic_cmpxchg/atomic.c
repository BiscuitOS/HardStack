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
 * atomic_xor_* (ARMv7 Cotex-A9MP)
 *
 * static inline int atomic_cmpxchg_relaxed(atomic_t *ptr, int old, int new)
 * {
 *         int oldval;
 *         unsigned long res;
 * 
 *         prefetchw(&ptr->counter);
 * 
 *         do {
 *                 __asm__ __volatile__("@ atomic_cmpxchg\n"
 *                 "ldrex  %1, [%3]\n"
 *                 "mov    %0, #0\n"
 *                 "teq    %1, %4\n"
 *                 "strexeq %0, %5, [%3]\n"
 *                     : "=&r" (res), "=&r" (oldval), "+Qo" (ptr->counter)
 *                     : "r" (&ptr->counter), "Ir" (old), "r" (new)
 *                     : "cc");
 *         } while (res);
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

	/* Atomic cmpxchg: Old == original */
	val = atomic_cmpxchg(&BiscuitOS_counter, 8, 9);

	printk("[0]Atomic: oiginal-> %d new-> %d\n", val, 
			atomic_read(&BiscuitOS_counter));

	/* Atomic cmpxchg: Old != original */
	val = atomic_cmpxchg(&BiscuitOS_counter, 1, 5);

	printk("[1]Atomic: original-> %d new-> %d\n", val, 
			atomic_read(&BiscuitOS_counter));

	return 0;
}
device_initcall(atomic_demo_init);
