/*
 * Kernel notifier
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define BISCUITOS_EVENT_A		0x01
#define BISCUITOS_EVENT_B		0x02

static RAW_NOTIFIER_HEAD(BiscuitOS_chain);

int BiscuitOS_notifier_event(struct notifier_block *nb, 
					unsigned long event, void *v)
{
	switch (event) {
	case BISCUITOS_EVENT_A:
		printk("BiscuitOS notifier event A [%s]\n", (char *)v);
		break;
	case BISCUITOS_EVENT_B:
		printk("BiscuitOS notifier event B [%s]\n", (char *)v);
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block BiscuitOS_notifier = {
	.notifier_call = BiscuitOS_notifier_event,
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	printk("Hello modules on BiscuitOS\n");

	/* Register notifier */
	raw_notifier_chain_register(&BiscuitOS_chain, &BiscuitOS_notifier);

	/* Notifier */
	raw_notifier_call_chain(&BiscuitOS_chain, 
					BISCUITOS_EVENT_B, "BiscuitOS");
	raw_notifier_call_chain(&BiscuitOS_chain,
					BISCUITOS_EVENT_A, "Buddy");

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	/* Unregister notifier */
	raw_notifier_chain_unregister(&BiscuitOS_chain, &BiscuitOS_notifier);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common Device driver on BiscuitOS");
