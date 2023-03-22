// SPDX-License-Identifier: GPL-2.0
/*
 * CACHE SCENE: FLUSH CACHE memcpy
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	void *dst, *src;

	dst = (void *)__get_free_page(GFP_KERNEL);
	src = (void *)__get_free_page(GFP_KERNEL);
	if (!dst || !src) {
		printk("System Error: No free memory!\n");
		return -ENOMEM;
	}
	sprintf((char *)src, "Hello BiscuitOS");

	memcpy_flushcache(dst, src, strlen(src));

	printk("DST: %s\n", (char *)dst);

	free_page((unsigned long)src);
	free_page((unsigned long)dst);

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("memcpy");
