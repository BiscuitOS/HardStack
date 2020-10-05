/*
 * BiscuitOS Kernel BiscuitOS Code
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>

/* Shrink interface */
#include <linux/mm.h>
#include <linux/shrinker.h>

extern void drop_slab(void);
unsigned long BiscuitOS_free_pages = 0x1000;
unsigned long BiscuitOS_used_pages = 0x200;

static unsigned long
mmu_shrink_count_bs(struct shrinker *shrink, struct shrink_control *sc)
{
	printk("BiscuitOS Count...\n\n\n");
	return BiscuitOS_used_pages;
}

static unsigned long
mmu_shrink_scan_bs(struct shrinker *shrink, struct shrink_control *sc)
{
	printk("BiscuitOS Scan... \n\n\n");
	return BiscuitOS_free_pages;
}

static struct shrinker mmu_shrinker_bs = {
	.count_objects = mmu_shrink_count_bs,
	.scan_objects  = mmu_shrink_scan_bs,
	.seeks = DEFAULT_SEEKS * 10,
};

/* Shrink */
static int __init BiscuitOS_shrink_init(void)
{
	register_shrinker(&mmu_shrinker_bs);

	return 0;
}

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	printk("Hello BiscuitOS on kernel.\n");

	drop_slab();

	/* unregister */
	unregister_shrinker(&mmu_shrinker_bs);
	return 0;
}

fs_initcall(BiscuitOS_shrink_init);
device_initcall(BiscuitOS_init);
