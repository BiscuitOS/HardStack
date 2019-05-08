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
 * static inline void arch_spin_lock(arch_spinlock_t *lock)
 * {
 *         unsigned long tmp;
 *         u32 newval;
 *         arch_spinlock_t lockval;
 * 
 *         prefetchw(&lock->slock);
 *         __asm__ __volatile__(
 * "1:     ldrex   %0, [%3]\n"
 * "       add     %1, %0, %4\n"
 * "       strex   %2, %1, [%3]\n"
 * "       teq     %2, #0\n"
 * "       bne     1b"
 *         : "=&r" (lockval), "=&r" (newval), "=&r" (tmp)
 *         : "r" (&lock->slock), "I" (1 << TICKET_SHIFT)
 *         : "cc");
 * 
 *         while (lockval.tickets.next != lockval.tickets.owner) {
 *                 wfe();
 *                 lockval.tickets.owner = READ_ONCE(lock->tickets.owner);
 *         }
 *  
 *         smp_mb();
 * }
 */

static spinlock_t BiscuitOS_lock;

static __init int spinlock_demo_init(void)
{
	/* Initialize spinlock */
	spin_lock_init(&BiscuitOS_lock);

	/* acquire lock */
	spin_lock(&BiscuitOS_lock);

	__asm__ volatile ("nop");

	/* release lock */
	spin_unlock(&BiscuitOS_lock);

	printk("Spinlock done.....\n");

	return 0;
}
device_initcall(spinlock_demo_init);
