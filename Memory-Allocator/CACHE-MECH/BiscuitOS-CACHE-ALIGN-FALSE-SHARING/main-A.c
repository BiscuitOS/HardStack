// SPDX-License-Identifier: GPL-2.0
/*
 * CACHE Align: False Sharing
 *
 * (C) 2023.05.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>

/*
 * +-----+-----+-------------------------------+
 * |  A  |  B  |                               | CACHE Line N
 * +-----+-----+-------------------------------+
 */
struct BiscuitOS_Adata
{
	volatile unsigned long A;
	volatile unsigned long B;
} __attribute__((packed));

/* Global Data */
struct BiscuitOS_Adata node;
/* Local Data */
static struct task_struct *kp0, *kp1;
/* Loop Time */
#define LOOP		1000

static int kthread_fun0(void *data)
{
	unsigned long count = LOOP;
	unsigned long rdta, rdtb;

	rdta = rdtsc();
	while (count--) {
		/* RMW */
		node.A = 0x10;
	}

	rdtb = rdtsc();
	printk(" Kthread0 Running on %d\n cost: %#lx\n",
				smp_processor_id(), rdtb - rdta);
	kthread_stop(kp1);

	return 0;
}

static int kthread_fun1(void *data)
{
	printk(" Kthread1 Running on %d\n", smp_processor_id());

	while (!kthread_should_stop()) {
		/* RMW */
		node.B = 0x10;
	}
	return 0;
}

static int __init BiscuitOS_init(void)
{
	printk("CACHE Alignment A:\n A: %#lx\n B: %#lx\n",
				(unsigned long)&node.A,
				(unsigned long)&node.B);

	kp0 = kthread_create(kthread_fun0, NULL, "Kthread0");
	if (!kp0) {
		printk("System Error: Create kthread0 failed.\n");
		return -EINVAL;
	}
	kp1 = kthread_create(kthread_fun1, NULL, "Kthread1");
	if (!kp1) {
		printk("System Error: Create kthread1 failed.\n");
		kthread_stop(kp0);
	}

	/* Running on Special CPU */
	kthread_bind(kp0, 0);
	kthread_bind(kp1, 2);
	wake_up_process(kp1);
	mdelay(100);
	wake_up_process(kp0);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("CACHE Align: False Sharing");
