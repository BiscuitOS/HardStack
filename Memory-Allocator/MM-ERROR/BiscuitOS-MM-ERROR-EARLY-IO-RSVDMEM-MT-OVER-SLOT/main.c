/*
 * EARLY_IOREMAP Memory Allocator BUG
 *  - No free slot
 *
 * (C) 2022.10.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/fixmap.h>
#include <asm-generic/early_ioremap.h>

#define BROILER_MMIO_BASE	0xD0000000
#define BROILER_MMIO_SIZE	0x1000

int __init BiscuitOS_Running(void)
{
	void __iomem *mmio;
	int i;

	for (i = 0; i < FIX_BTMAPS_SLOTS; i++) {
		/* mapping */
		mmio = early_ioremap(BROILER_MMIO_BASE + i * BROILER_MMIO_SIZE, 
							     BROILER_MMIO_SIZE);
		if (!mmio) {
			printk("EARLY-IOREMAP failed on %d\n", i);
			return -ENOMEM;
		}
	}

	return 0;
}
