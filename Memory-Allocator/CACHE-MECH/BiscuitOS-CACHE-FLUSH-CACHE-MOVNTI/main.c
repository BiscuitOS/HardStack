// SPDX-License-Identifier: GPL-2.0
/*
 * FLUSH CACHE on Non-Temporal Hit
 *
 * (C) 2023.04.03 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/cacheflush.h>
#include <linux/slab.h>

static int __init BiscuitOS_init(void)
{
	void *src, *dst;

	/* DST Memory */
	dst = (void *)__get_free_page(GFP_KERNEL);
	if (!dst) {
		printk("System Error: No free memory on Buddy\n");
		return -ENOMEM;
	}
	memset(dst, 0x00, PAGE_SIZE);

	/* SRC Memory */
	src = kmalloc(PAGE_SIZE / 8, GFP_KERNEL);
	if (!src) {
		printk("System Error: No free memory on Slab\n");
		free_page((unsigned long)dst);
		return -ENOMEM;
	}
	sprintf((char *)src, "Hello BiscuitOS");

	/* COPY and Flush CACHE */
	__memcpy_flushcache(dst, src, PAGE_SIZE / 8);

	/* Check DST */
	printk("%s\n", (char *)dst);

	/* free */
	kfree(src);
	free_page((unsigned long)dst);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("MOVNTI on BiscuitOS");
