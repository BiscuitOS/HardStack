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

/*
 * ARMv7 arch_spin_lock()
 *
 * static inline int arch_spin_trylock(arch_spinlock_t *lock)
 * {
 *         unsigned long contended, res;
 *         u32 slock;
 *
 *         prefetchw(&lock->slock);
 *         do {
 *                 __asm__ __volatile__(
 *                 "       ldrex   %0, [%3]\n"
 *                 "       mov     %2, #0\n"
 *                 "       subs    %1, %0, %0, ror #16\n"
 *                 "       addeq   %0, %0, %4\n"
 *                 "       strexeq %2, %0, [%3]"
 *                 : "=&r" (slock), "=&r" (contended), "=&r" (res)
 *                 : "r" (&lock->slock), "I" (1 << TICKET_SHIFT)
 *                 : "cc");
 *         } while (res);
 *
 *         if (!contended) {
 *                 smp_mb();
 *                 return 1;
 *         } else {
 *                 return 0;
 *         }
 * }
 */

static spinlock_t BiscuitOS_lock;

static __init int spinlock_demo_init(void)
{
	/* Initialize spinlock */
	spin_lock_init(&BiscuitOS_lock);

	/* try acquire lock */
	if (spin_trylock(&BiscuitOS_lock)) {

		__asm__ volatile ("nop");

		/* release lock */
		spin_unlock(&BiscuitOS_lock);
	} else {
		printk("Unable obtain spinlock.\n");
	}

	printk("Spinlock done.....\n");

	return 0;
}
device_initcall(spinlock_demo_init);
