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
 * atomic_xchg (ARMv7 Cotex-A9MP)
 *
 * static inline unsigned long __xchg(unsigned long x, volatile void *ptr,
 *						int size)
 * {
 * 	unsigned long ret;
 * 	unsigned int tmp;
 *
 * 	asm volatile("@ __xchg4\n"
 * 	"1:     ldrex   %0, [%3]\n"
 * 	"       strex   %1, %2, [%3]\n"
 * 	"       teq     %1, #0\n"
 * 	"       bne     1b"
 * 		: "=&r" (ret), "=&r" (tmp)
 * 		: "r" (x), "r" (ptr)
 * 		: "memory", "cc");
 *
 *	return ret;
 * }
 */

#include <linux/kernel.h>
#include <linux/init.h>

static atomic_t BiscuitOS_counter = ATOMIC_INIT(8);

/* atomic_* */
static __init int atomic_demo_init(void)
{
	/* Atomic value before exhange data  */
	printk("[0]Atomic: oiginal-> %d\n", atomic_read(&BiscuitOS_counter));

	/* Atomic xhg */
	atomic_xchg(&BiscuitOS_counter, 6);

	/* Atomic value after exhange data  */
	printk("[1]Atomic: new->     %d\n", atomic_read(&BiscuitOS_counter));

	return 0;
}
device_initcall(atomic_demo_init);
