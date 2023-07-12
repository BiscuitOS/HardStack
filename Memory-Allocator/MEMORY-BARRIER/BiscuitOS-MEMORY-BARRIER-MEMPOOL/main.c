// SPDX-License-Identifier: GPL-2.0
/*
 * MEMORY BARRIER: MEMPOOL
 *
 * (C) 2023.07.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mempool.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#define MIN_NR		4
static mempool_t *pool;
static struct kmem_cache *buffer;
static struct task_struct *tsk1, *tsk2;

static int processor_func(void *info)
{
	while (!kthread_should_stop()) {
		void *data = mempool_alloc(pool, GFP_KERNEL);

		if (!data) {
			printk("Alloc Failed.\n");
			mdelay(1000);
			continue;
		}
	
		sprintf((char *)data, "Hello BiscuitOS: %s", (char *)info);
		printk("%s\n", (char *)data);
		if (strcmp((char *)info, "KA") == 0)
			mdelay(500);
		else
			mdelay(200);

		mempool_free(data, pool);
	}
	return 0;
}

static int __init BiscuitOS_init(void)
{
	buffer = kmem_cache_create("MEMPOOL", 64, 0, 0, NULL);
	if (!buffer) {
		printk("ERROR: Create KMEM\n");
		return -ENOMEM;
	}

	pool = mempool_create_slab_pool(MIN_NR, buffer);
	if (!pool) {
		printk("ERRORY: Create Pool Failed.\n");
		kmem_cache_destroy(buffer);
		return -ENOMEM;
	}

	tsk1 = kthread_create(processor_func, "KA", "thread1");
	tsk2 = kthread_create(processor_func, "KB", "thread2");

	wake_up_process(tsk1);
	wake_up_process(tsk2);

	mdelay(5000);
	kthread_stop(tsk1);
	kthread_stop(tsk2);

	mempool_destroy(pool);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("MEMORY BARRIER on BiscuitOS");
