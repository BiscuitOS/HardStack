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
 * atomic_xor (ARMv7 Cotex-A9MP)
 *
 * static inline void atomic_xor(int i, atomic_t *v)
 * {
 *         unsigned long tmp;
 *         int result;
 *
 *         prefetchw(&v->counter);
 *         __asm__ volatile ("\n\t"
 *         "@ atomic_xor\n\t"
 * "1:      ldrex   %0, [%3]\n\t"        @ result, tmp115
 * "        eor     %0, %0, %4\n\t"      @ result,
 * "        strex   %1, %0, [%3]\n\t"    @ tmp, result, tmp115
 * "        teq     %1, #0\n\t"          @ tmp
 * "        bne     1b"
 *          : "=&r" (result), "=&r" (tmp), "+Qo" (v->counter)
 *          : "r" (&v->counter), "Ir" (i)
 *          : "cc");
 * }
 */

#include <linux/kernel.h>
#include <linux/init.h>

static atomic_t BiscuitOS_counter = ATOMIC_INIT(7);

/* atomic_* */
static __init int atomic_demo_init(void)
{
	/* Atomic xor */
	atomic_xor(7, &BiscuitOS_counter);

	printk("Atomic: %d\n", atomic_read(&BiscuitOS_counter));

	return 0;
}
device_initcall(atomic_demo_init);
