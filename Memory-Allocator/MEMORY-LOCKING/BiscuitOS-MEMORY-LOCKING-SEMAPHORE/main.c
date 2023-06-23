// SPDX-License-Identifier: GPL-2.0
/*
 * LOCKING Mechanism on BiscuitOS: Semaphore
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/semaphore.h>

static struct task_struct *tsk1;
static struct task_struct *tsk2;
static struct task_struct *tsk3;

static struct semaphore sem;

static int kthread_fun(void *data)
{
	printk("%s Invoking.\n", (char *)data);
	down(&sem);
	printk("%s DOWN\n", (char *)data);
	while (!kthread_should_stop()) {
		; /* Do nothing */
	}
	up(&sem);
	printk("%s UP\n", (char *)data);
	return 0;
}

static int __init BiscuitOS_init(void)
{
	/* Initiliaze Semaphore */
	sema_init(&sem, 2); /* Only Alloc 2 Kthread Running */

	tsk1 = kthread_create(kthread_fun, "Kthread1", "Kthread1");
	tsk2 = kthread_create(kthread_fun, "Kthread2", "Kthread2");
	tsk3 = kthread_create(kthread_fun, "Kthread3", "Kthread3");

	wake_up_process(tsk1);
	mdelay(2000); /* Force Kthread1 Running */
	wake_up_process(tsk2);
	wake_up_process(tsk3);

	printk("Wakeup finish...\n");
	/* STOP Kthread */
	mdelay(3000);
	kthread_stop(tsk1);
	mdelay(2000);
	kthread_stop(tsk2);
	kthread_stop(tsk3);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common LOCKING on BiscuitOS");
