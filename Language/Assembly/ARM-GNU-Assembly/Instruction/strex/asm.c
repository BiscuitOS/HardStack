/*
 * strex
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

static volatile unsigned long R0 = 0;

static __init int strex_demo_init(void)
{
	unsigned long tmp;
	int result;

	__asm__ volatile ("nop\n\t"
			"ldrex	%0, [%3]\n\t"
			"add	%0, %0, #9\n\t"
			"strex	%1, %0, [%3]"
			: "=&r" (result), "=&r" (tmp), "+Qo" (R0)
			: "r" (&R0)
			: "cc");
	
	printk("R0: %#lx - result: %d - tmp: %#lx\n", R0, result, tmp);

	return 0;
}
device_initcall(strex_demo_init);
