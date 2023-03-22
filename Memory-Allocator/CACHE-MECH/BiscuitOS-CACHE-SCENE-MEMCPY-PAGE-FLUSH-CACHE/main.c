// SPDX-License-Identifier: GPL-2.0
/*
 * CACHE SCENE: FLUSH CACHE memcpy page
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	struct page *page;
	void *dst, *src;

	dst = (void *)__get_free_page(GFP_KERNEL);
	src = (void *)__get_free_page(GFP_KERNEL);
	if (!dst || !src) {
		printk("System Error: No free memory!\n");
		return -ENOMEM;
	}
	sprintf((char *)src, "Hello BiscuitOS");
	page = pfn_to_page(PHYS_PFN(__pa(src)));

	memcpy_page_flushcache(dst, page, 0, strlen(src));

	printk("DST: %s\n", (char *)dst);

	free_page((unsigned long)src);
	free_page((unsigned long)dst);

	return 0;
}
device_initcall(BiscuitOS_init);
