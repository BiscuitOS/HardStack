/*
 * Early IO/RSVD-MEM BUG
 *  - Use after Allocator destroy
 *  - echo 1 > /proc/sys/BiscuitOS/BiscuitOS-early-iomem
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sysctl.h>
#include <asm-generic/early_ioremap.h>

#define BROILER_MMIO_BASE	0xD0000000
#define BROILER_MMIO_SIZE	0x1000

static int BiscuitOS_early_iomem(struct ctl_table *table, int write,
                void __user *buffer, size_t *length, loff_t *ppos)
{
	void __iomem *mmio;

	/* mapping */
	mmio = early_ioremap(BROILER_MMIO_BASE, BROILER_MMIO_SIZE);
	if (!mmio) {
		printk("EARLY-IOREMAP failed\n");
		return -ENOMEM;
	}

	printk("Broiler MMIO: %#x\n", *(unsigned int *)mmio);

	/* unmapping */
	early_iounmap(mmio, BROILER_MMIO_SIZE);
	
	return 0;
}

static struct ctl_table BiscuitOS_table[] = {
	{
		.procname       = "BiscuitOS-early-iomem",
		.maxlen         = sizeof(int),
		.mode           = 0644,
		.proc_handler   = BiscuitOS_early_iomem,
	},
	{ }
};

static struct ctl_table sysctl_BiscuitOS_table[] = {
	{
		.procname       = "BiscuitOS",
		.mode           = 0555,
		.child          = BiscuitOS_table,
		},
	{ }
};

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	register_sysctl_table(sysctl_BiscuitOS_table);
	return 0;
}

device_initcall(BiscuitOS_init);
