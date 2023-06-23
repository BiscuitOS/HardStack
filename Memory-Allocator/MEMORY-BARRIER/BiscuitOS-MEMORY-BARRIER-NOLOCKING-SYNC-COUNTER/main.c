// SPDX-License-Identifier: GPL-2.0
/*
 * MEMORY BARRIER: Nolocking Synchronization(3)
 *
 * (C) 2023.06.10 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/spinlock.h>

#define NODE_INITIALIZED		0x00000020

static struct task_struct *tsk1;
static struct task_struct *tsk2;
static struct task_struct *tsk3;

struct node {
	unsigned int flags;
	atomic_t counter;
};
static struct node node;
static DEFINE_SPINLOCK(lock);
unsigned long bugs;

static int producer1_thread(void *info)
{
	while (!kthread_should_stop()) {
		spin_lock(&lock);
		/* Modify */
		atomic_set(&node.counter, 1);
		node.flags |= NODE_INITIALIZED;
		spin_unlock(&lock);
	}

	return 0;
}

static int producer2_thread(void *info)
{
	while (!kthread_should_stop()) {
		spin_lock(&lock);
		/* CLEAR */
		node.flags = 0;
		atomic_set(&node.counter, 0);
		spin_unlock(&lock);
	}

	return 0;
}

static int consumer_thread(void *data)
{
	while (!kthread_should_stop()) {
		unsigned int counter;

		smp_mb();
		counter = atomic_read(&node.counter);

		smp_rmb();
		if ((!counter && node.flags))
			bugs++;

		mdelay(100);
	}

	return 0;
}

static int __init BiscuitOS_init(void)
{

	tsk1 = kthread_create(producer1_thread, "Producer", "Kthread1");
	tsk2 = kthread_create(producer2_thread, "Producer", "Kthread2");
	tsk3 = kthread_create(consumer_thread, "Consumer", "Kthread3");

	wake_up_process(tsk1);
	wake_up_process(tsk2);
	wake_up_process(tsk3);

	printk("Wakeup Finish.....\n");
	mdelay(5000);
	/* STOP Kthread */
	kthread_stop(tsk1);
	kthread_stop(tsk2);
	kthread_stop(tsk3);
	printk("BUGs: %#lx\n", bugs);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common Memory Barriers on BiscuitOS");
