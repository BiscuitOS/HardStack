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

#include <linux/fs.h>
#include <asm/uaccess.h>

/*
 * Memory region for Hotplugging.
 * BTW, The default Block size as 0x8000000, not 2 Gig.
 *
 * On Aarch, we reserved a memory region from 0x50000000 to 0x58000000 on
 * MEMBLOCK allocator that invoke "memblock_reserve(0x50000000, 0x8000000)"
 * on "arm64_memblock_init()".
 *
 * On x86_64, we can use "memmap=" on CMDLINE to reserved a memory
 * region. The range of memory region from 0x5000000 to 0xD000000, which 
 * descrbe as "memmap=0x128M$0x5000000".
 */

/* NUMA NODE */
static int nid = 0;

#ifdef __aarch64__
#define BISCUITOS_MEMORY_BASE	0x50000000
#define BISCUITOS_PATH		"/sys/devices/system/memory/memory10/state"
#else // X64
#define BISCUITOS_MEMORY_BASE	0x18000000
#define BISCUITOS_PATH		"/sys/devices/system/memory/memory3/state"
#endif
#define BISCUITOS_MEMORY_SIZE	0x8000000

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	struct file *filp;
	mm_segment_t fs;
	loff_t pos = 0;

#ifdef CONFIG_MEMORY_HOTPLUG
	/* Add into /sys/device/system/memory/memoryX */
	add_memory(nid, BISCUITOS_MEMORY_BASE, BISCUITOS_MEMORY_SIZE);
#endif

	/* open file */
	filp = filp_open(BISCUITOS_PATH, O_RDWR, 0644);
	if (IS_ERR(filp)) {
		printk("ERROR[%ld]: open %s failed.\n", PTR_ERR(filp),
							BISCUITOS_PATH);
		return PTR_ERR(filp);
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	/* Write */
	pos = 0;
	kernel_write(filp, "online", strlen("online"), &pos);

	/* Close */
	filp_close(filp, NULL);
	set_fs(fs);

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
