/*
 * Memory Hotplug in Code on BiscuitOS
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

/*
 * Memory region for Hotplugging.
 * BTW, The default Block size as 0x8000000, not 2 Gig.
 *
 * On Aarch, we reserved a memory region from 0x50000000 to 0x58000000 on
 * MEMBLOCK allocator that invoke "memblock_reserve(0x50000000, 0x8000000)"
 * on "arm64_memblock_init()".
 *
 * On x86_64, we can use "memmap=" on CMDLINE to reserved a memory
 * region. The range of memory region from 0x18000000 to 0x20000000, which 
 * descrbe as "memmap=0x128M$0x18000000".
 */

/* NUMA NODE */
static int nid = 0;

#ifdef __aarch64__
#define BISCUITOS_MEMORY_BASE		0x50000000
#else // X64
#define BISCUITOS_MEMORY_BASE		0x18000000
#endif
#define BISCUITOS_MEMORY_SIZE		0x8000000

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
#ifdef CONFIG_MEMORY_HOTPLUG
	/* Add into /sys/device/system/memory/memoryX */
	add_memory(nid, BISCUITOS_MEMORY_BASE, BISCUITOS_MEMORY_SIZE);
#endif
	printk("Hello BiscuitOS :)\n");
	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
#ifdef CONFIG_MEMORY_HOTPLUG
	remove_memory(nid, BISCUITOS_MEMORY_BASE, BISCUITOS_MEMORY_SIZE);
#endif
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Memory Hotplug in Code on BiscuitOS");
