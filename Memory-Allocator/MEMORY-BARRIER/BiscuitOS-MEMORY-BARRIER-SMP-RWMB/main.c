// SPDX-License-Identifier: GPL-2.0
/*
 * MEMORY BARRIER: Read/Write Barrier
 *
 * (C) 2023.06.10 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

#define RING_SIZE		128

static struct task_struct *tsk1;
static struct task_struct *tsk2;

struct RingBuffer {
	int data[RING_SIZE];
	int head;
	int tail;
};
static struct RingBuffer buf;

static int producer_thread(void *data)
{
	while (!kthread_should_stop()) {
		int i;

		for (i = 0; i < 100; i++) {
			while ((buf.head + 1) % RING_SIZE == buf.tail) {
				/* RingBuffer is full, wait */
				mdelay(10);
			}

			buf.data[buf.head] = i;
			/*
			 * Make sure the data is written before updating
			 * the head.
			 */
			smp_wmb();
			buf.head = (buf.head + 1) % RING_SIZE;
			mdelay(50);
		}
	}

	return 0;
}

static int consumer_thread(void *data)
{
	while (!kthread_should_stop()) {
		if (buf.tail == buf.head) {
			/* Ringbuffer is empty, wait */
			mdelay(10);
			continue;
		}

		/* Ensure see the updated data after seeing the updated head */
		smp_rmb();
		printk("Consumed: %d\n", buf.data[buf.tail]);
		buf.tail = (buf.tail + 1) % RING_SIZE;
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
	mdelay(3000);
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
