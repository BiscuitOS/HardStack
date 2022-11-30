/*
 * MEMBLOCK Hotplug Memory
 *
 * (C) 2022.10.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memblock.h>
#include <linux/ioport.h>

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	struct memblock_region *region;
	char *mem = NULL;

	/* Iterate Driver Managed Memory */
	printk("=== MEMBLOCK Hotplugable ===\n");
	for_each_mem_region(region)
		if (memblock_is_hotpluggable(region)) {
			printk("RANGE: %#llx - %#llx - NID %d\n", region->base,
				region->base + region->size, region->nid);
			mem = (char *)phys_to_virt(region->base);
		}

	/* Driver Manage Memory..... Ignore details */
	if (mem) {
		sprintf(mem, "Hello %s", "BiscuitOS");
		printk("== %s ==\n", mem);
	}

	return 0;
}
device_initcall(BiscuitOS_init);
