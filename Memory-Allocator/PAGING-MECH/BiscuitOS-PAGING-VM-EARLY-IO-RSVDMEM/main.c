// SPDX-License-Identifier: GPL-2.0
/*
 * EARLY MMIO/RSVDMEM Mapping: RSVDMEM
 *
 *   CMDLINE ADD "memmap=4K$0x10000000"
 *
 * (C) 2023.11.29 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm-generic/early_ioremap.h>

#define RSVDMEM_BASE	0x10000000
#define RSVDMEM_SIZE	0x1000

int __init BiscuitOS_Running(void)
{
	void __iomem *mmio;

	/* MAPPING */
	mmio = early_ioremap(RSVDMEM_BASE, RSVDMEM_SIZE);
	if (!mmio) {
		printk("EARLY-IOREMAP failed\n");
		return -ENOMEM;
	}

	/* ACCESSED */
	printk("RSVDMEM: %#x\n", *(unsigned int *)mmio);

	/* UNMAPPING */
	early_iounmap(mmio, RSVDMEM_SIZE);

	return 0;
}
