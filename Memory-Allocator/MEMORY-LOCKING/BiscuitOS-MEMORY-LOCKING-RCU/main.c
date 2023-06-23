// SPDX-License-Identifier: GPL-2.0
/*
 * LOCKING Mechanism on BiscuitOS: RCU
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>

struct BiscuitOS_node {
	int value;
};
static struct __rcu BiscuitOS_node *bnode;

static struct task_struct *tsk1;
static struct task_struct *tsk2;
static struct task_struct *tsk3;

static int kthread_read(void *data)
{
	struct BiscuitOS_node *bnp;

	printk("%s Invoking.\n", (char *)data);
	while (!kthread_should_stop()) {
		rcu_read_lock();
		printk("%s RCU-RLOCK\n", (char *)data);
		bnp = rcu_dereference(bnode);

		if (bnp)
			printk("%s READ %d\n", (char *)data, bnp->value);
		printk("%s RCU-RUNLOCK\n", (char *)data);
		rcu_read_unlock();
		mdelay(1000);
	}
	return 0;
}

static int kthread_write(void *data)
{
	struct BiscuitOS_node *new, *old;

	printk("%s Invoking.\n", (char *)data);
	while (!kthread_should_stop()) {
		printk("%s RCU-WLOCK\n", (char *)data);
		new = kmalloc(sizeof(*new), GFP_KERNEL);
		if (!new)
			return -ENOMEM;
		new->value = jiffies;

		old = rcu_replace_pointer(bnode, new, 1);
		synchronize_rcu();
		printk("%s RCU-WUNLOCK %d\n", (char *)data, new->value);
		kfree(old);

		mdelay(1000);
	}
	return 0;
}

static int __init BiscuitOS_init(void)
{
	tsk1 = kthread_create(kthread_write, "Kthread1", "Kthread1");
	tsk2 = kthread_create(kthread_read, "Kthread2", "Kthread2");
	tsk3 = kthread_create(kthread_read, "Kthread3", "Kthread3");

	wake_up_process(tsk1);
	wake_up_process(tsk2);
	wake_up_process(tsk3);

	printk("Wakeup finish...\n");
	/* STOP Kthread */
	mdelay(4000);
	kthread_stop(tsk1);
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
