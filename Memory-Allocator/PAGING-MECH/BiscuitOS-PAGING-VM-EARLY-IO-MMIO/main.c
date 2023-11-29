// SPDX-License-Identifier: GPL-2.0
/*
 * EARLY MMIO/RSVDMEM Mapping: MMIO
 *
 * (C) 2023.11.29 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm-generic/early_ioremap.h>

#define BROILER_MMIO_BASE	0xD0000000
#define BROILER_MMIO_SIZE	0x1000

int __init BiscuitOS_Running(void)
{
	void __iomem *mmio;

	/* MAPPING */
	mmio = early_ioremap(BROILER_MMIO_BASE, BROILER_MMIO_SIZE);
	if (!mmio) {
		printk("EARLY-IOREMAP failed\n");
		return -ENOMEM;
	}

	printk("Broiler MMIO: %#x\n", *(unsigned int *)mmio);

	/* UNMAPPING */
	early_iounmap(mmio, BROILER_MMIO_SIZE);

	return 0;
}
