/*
 * Uncache with Memory
 *
 * (C) 2023.01.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/io.h>

#define BROILER_MEM_BASE	0x20000000UL
#define BROILER_MEM_LEN		0x1000UL

static void __iomem *mem;

static int __init BiscuitOS_init(void)
{
	unsigned long *val;

	/* IOREMAP with UNCACHE */
	mem = ioremap_uc(BROILER_MEM_BASE, BROILER_MEM_LEN);
	if (!mem) {
		printk("IOREMAP MMIO failed.\n");
		return -EINVAL;
	}
	
	/* Memory Read and Write */
	val = (unsigned long *)mem;
	*val = 0x88520;
	printk("MMIO: Phys %#lx - %#lx\n",
			BROILER_MEM_BASE, BROILER_MEM_BASE + BROILER_MEM_LEN);
	printk("      Virt %#lx - %#lx\n", (unsigned long)mem, 
				(unsigned long)mem + BROILER_MEM_LEN);
	printk("      Value: %#lx\n", *val);

	return 0;
}

static void __exit BiscuitOS_exit(void)
{
	iounmap(mem);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Uncache with Memory on BiscuitOS");
