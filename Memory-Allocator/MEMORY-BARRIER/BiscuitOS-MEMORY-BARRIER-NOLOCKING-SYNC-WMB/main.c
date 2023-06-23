// SPDX-License-Identifier: GPL-2.0
/*
 * MEMORY BARRIER: Nolocking Synchronization(2)
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

struct node {
	struct node *next;
	unsigned long *ptr;
	unsigned long *base;
	unsigned long flags;
};
static struct node nodelist;
static DEFINE_SPINLOCK(lock);

static int producer_thread(void *info)
{
	while (!kthread_should_stop()) {
		struct node *np = kzalloc(sizeof(*np), GFP_KERNEL);

		if (!np)
			continue;
		spin_lock(&lock);
		np->ptr = kmalloc(sizeof(unsigned long), GFP_KERNEL);
		np->base = kmalloc(sizeof(unsigned long), GFP_KERNEL);

		smp_wmb();
		np->flags |= NODE_INITIALIZED;

		np->next = nodelist.next;
		nodelist.next = np;
		spin_unlock(&lock);

		mdelay(500);
	}

	return 0;
}

static int consumer_thread(void *data)
{
	while (!kthread_should_stop()) {
		struct node *np;

		if (nodelist.next)
			printk("Travers List:\n");
		for (np = nodelist.next; np; np = np->next) {
			if (!(np->flags & NODE_INITIALIZED))
				continue;
			printk("PTR: %p BASE: %p\n", np->ptr, np->base);
		}
		mdelay(1000);
	}

	return 0;
}

static int __init BiscuitOS_init(void)
{

	tsk1 = kthread_create(producer_thread, "Producer", "Kthread1");
	tsk2 = kthread_create(consumer_thread, "Consumer", "Kthread2");

	wake_up_process(tsk1);
	wake_up_process(tsk2);

	printk("Wakeup Finish.....\n");
	mdelay(5000);
	/* STOP Kthread */
	kthread_stop(tsk1);
	kthread_stop(tsk2);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common Memory Barriers on BiscuitOS");
