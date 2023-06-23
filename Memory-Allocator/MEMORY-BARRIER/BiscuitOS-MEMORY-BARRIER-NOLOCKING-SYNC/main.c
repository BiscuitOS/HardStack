// SPDX-License-Identifier: GPL-2.0
/*
 * MEMORY BARRIER: Nolocking Synchronization
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
	unsigned long *data;
	int flags;
};
static struct node nodelist;
static DEFINE_SPINLOCK(lock);
static unsigned long count;

static int producer_thread(void *info)
{
	while (!kthread_should_stop()) {
		struct node *node = kzalloc(sizeof(*node), GFP_KERNEL);

		if (!node) {
			printk("SystemError: No free memory.\n");
			mdelay(100);
			continue;
		}

		spin_lock(&lock);
		node->next = nodelist.next;
		nodelist.next = node;
		spin_unlock(&lock);

		node->data = kmalloc(sizeof(int), GFP_KERNEL);
		*node->data = ++count; /* CACHE Line Write */

		/* Emulate Race condition */
		mdelay(1000);

		smp_wmb();
		node->flags |= NODE_INITIALIZED; /* CACHE Line Write */
		mdelay(100);
	}

	return 0;
}

static int consumer_thread(void *data)
{
	while (!kthread_should_stop()) {
		struct node *np;

		spin_lock(&lock);
		np = nodelist.next;
		spin_unlock(&lock);

		if (np)
			printk("Traverse nodelist:\n");
		for (; np; np = np->next) {
			/* CACHE Line Read */
			if (!(np->flags & NODE_INITIALIZED))
				continue;

			smp_rmb();
			/* CACHE Line Read */
			printk("DATA: %#lx\n", *np->data);
		}
		mdelay(500);
	}

	return 0;
}

static int __init BiscuitOS_init(void)
{

	tsk1 = kthread_create(producer_thread, "Producer", "Kthread1");
	tsk2 = kthread_create(consumer_thread, "Consumer", "Kthread2");

	kthread_bind(tsk1, 1);
	kthread_bind(tsk2, 2);
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
