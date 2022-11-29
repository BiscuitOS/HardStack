/*
 * MEMBLOCK DRIVER_MANAGED
 *
 * - Must add 'memmap=${BISCUITOS_FAKE_SIZE}$${BISCUITOS_FAKE_BASE}'
 *   - 'memmap=128M$0x10000000' on CMDLINE
 * - Must enable CONFIG_ARCH_KEEP_MEMBLOCK
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
#include <linux/memory_hotplug.h>

#define BISCUITOS_FAKE_BASE	0x10000000
#define BISCUITOS_FAKE_SIZE	0x8000000
#define BISCUITOS_FAKE_END	(BISCUITOS_FAKE_BASE + BISCUITOS_FAKE_SIZE - 1)

static struct resource BiscuitOS_resource = {
	.start		= BISCUITOS_FAKE_BASE,
	.end		= BISCUITOS_FAKE_END,
	.flags		= IORESOURCE_SYSRAM_DRIVER_MANAGED,
	.name		= "System RAM Driver Managed",
};

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	struct memblock_region *region;

	/* Add RAM as Drvier managed */
	add_memory_resource(0, &BiscuitOS_resource, MHP_NONE);

	/* Iterate Driver Managed Memory */
	printk("=== MEMBLOCK Driver Managed ===\n");
	for_each_mem_region(region)
		if (memblock_is_driver_managed(region))
			printk("RANGE: %#llx - %#llx - NID %d\n", region->base,
				region->base + region->size, region->nid);

	/* Driver Manage Memory..... Ignore details */

	return 0;
}
device_initcall(BiscuitOS_init);
