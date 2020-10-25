/*
 * Anonymous mmap from kernel on BiscuitOS
 *
 * (C) 2020.10.24 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/mman.h>

/* The size for mmaping */
#define MMAP_SIZE		PAGE_SIZE
/* The address fro mamping */
static char *mmap_base;

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	/* Anonymous mmaping */
	mmap_base = (char *)vm_mmap(
			    NULL,
			    0,
			    MMAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED | MAP_ANONYMOUS,
			    0);
	if (IS_ERR((void *)mmap_base)) {
		printk("ERROR: Anonymous mmap failed.\n");
		return -ENOMEM;
	}

	/* Use */
	sprintf(mmap_base, "BiscuitOS");
	printk("=> %s [%#lx]\n", mmap_base, (unsigned long)mmap_base);

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	/* Un-mmap */
	vm_munmap((unsigned long)mmap_base, MMAP_SIZE);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Anonymous kenrel mmap on BiscuitOS");
