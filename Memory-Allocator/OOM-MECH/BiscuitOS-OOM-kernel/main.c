/*
 * OOM on Kernel Space (BiscuitOS+)
 *
 * (C) 2022.05.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) BiscuitOS
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/oom.h>

#define PAGE_OOM_BACKUP		20
#define PAGE_OOM_NOBACKUP	0

/* oom notifier */
static int BiscuitOS_oom_stop = 0;

/* caculate page number on speical order */
static unsigned long caculate_order_free_pages(int order)
{
	struct pglist_data *pgdat;
	unsigned long  free = 0;
	struct zone *zone;
	int idx, i, zone_type;

	for (zone_type = 0; zone_type < __MAX_NR_ZONES; zone_type++) {
		for_each_node_mask(idx, node_states[N_ONLINE]) {
			pgdat = NODE_DATA(idx);
			zone = &pgdat->node_zones[zone_type];

			if (!populated_zone(zone))
				continue;

			for (i = order; i < MAX_ORDER; i++)
				free += zone->free_area[i].nr_free << (i - order);
		}
	}
	return free;
}

/* cost free page */
static void cost_pages(unsigned long count, int order, int release)
{
	struct page *pages[PAGE_OOM_BACKUP + 1];
	int j, i = 0;

	while (count--) {
		pages[i] = alloc_pages(GFP_KERNEL, order);
		if (!pages[i])
			break;
		if (BiscuitOS_oom_stop) {
			for (j = 0; j < i; j++)
				__free_pages(pages[j], order);
			BiscuitOS_oom_stop = 0;
			break;
		}
		if (++i > release)
			i = release; /* forbid overflow */
	}
}

/* oom notify handler */
static int BiscuitOS_oom_notify(struct notifier_block *self,
				unsigned long notused, void *nfreed)
{
	printk("BiscuitOS OOM notifier accept.\n");
	BiscuitOS_oom_stop = 1;
	return 0;
}

static struct notifier_block BiscuitOS_oom_nb = {
	.notifier_call = BiscuitOS_oom_notify,
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	unsigned long count = 0;

	/* register oom notifier */
	register_oom_notifier(&BiscuitOS_oom_nb);

	/* Caculate free pages for special order */
	count = caculate_order_free_pages(PAGE_ALLOC_COSTLY_ORDER);

	/* Trigger oom */
	printk("***** BiscuitOS Trigger OOM once *****\n");
	printk("*****                            *****\n");
	printk("*****                            *****\n");
	cost_pages(count, PAGE_ALLOC_COSTLY_ORDER, PAGE_OOM_BACKUP);
	printk("***** BiscuitOS OOM Finish *****\n");

	/* unregister oom notifer */
	unregister_oom_notifier(&BiscuitOS_oom_nb);
	return 0;
	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("OOM on BiscuitOS");
