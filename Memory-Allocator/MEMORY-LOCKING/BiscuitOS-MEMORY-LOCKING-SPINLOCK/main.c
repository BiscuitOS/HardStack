// SPDX-License-Identifier: GPL-2.0
/*
 * LOCKING Mechanism on BiscuitOS: Spinlock
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

static struct task_struct *tsk1;
static struct task_struct *tsk2;

DEFINE_SPINLOCK(BiscuitOS_spinlock);

static int kthread_fun(void *data)
{
	printk("%s Invoking.\n", (char *)data);
	spin_lock(&BiscuitOS_spinlock);
	printk("%s Spinlock on CPU%d\n", (char *)data, smp_processor_id());
	while (!kthread_should_stop()) {
		; /* Do nothing */
	}
	printk("%s Spinunlock on CPU%d\n", (char *)data, smp_processor_id());
	spin_unlock(&BiscuitOS_spinlock);
	return 0;
}

static int __init BiscuitOS_init(void)
{

	tsk1 = kthread_create(kthread_fun, "Kthread-1", "Kthread1");
	tsk2 = kthread_create(kthread_fun, "Kthread-2", "Kthread2");

	kthread_bind(tsk1, 1);
	kthread_bind(tsk2, 2);
	wake_up_process(tsk1);
	mdelay(3000); /* Ensure invoke task1 */
	wake_up_process(tsk2);

	printk("Wakeup Finish.....\n");
	/* STOP Kthread */
	mdelay(3000);
	kthread_stop(tsk1);
	mdelay(2000);
	kthread_stop(tsk2);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common LOCKING on BiscuitOS");
