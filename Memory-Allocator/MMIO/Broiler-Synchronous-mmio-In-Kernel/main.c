/*
 * Broiler Synchronous MMIO In-Kernel on BiscuitOS
 *
 * (C) 2022.09.25 BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define BROILER_MMIO_BASE	0xD0000020
#define BROILER_MMIO_LEN	0x10
#define SLOT_NUM_REG		0x00
#define SLOT_SEL_REG		0x04
#define MIN_FREQ_REG		0x08
#define MAX_FREQ_REG		0x0C

static struct resource Broiler_pio_res = {
	.name	= "Broiler Synchronous MMIO In-Kernel",
	.start	= BROILER_MMIO_BASE,
	.end	= BROILER_MMIO_BASE + BROILER_MMIO_LEN,
	.flags	= IORESOURCE_MEM,
};
static void __iomem *mmio;

static int __init BiscuitOS_init(void)
{
	int r;
	uint32_t val;

	r = request_resource(&iomem_resource, &Broiler_pio_res);
	if (r < 0)
		return r;

	mmio = ioremap(BROILER_MMIO_BASE, BROILER_MMIO_LEN);
	if (!mmio) {
		r = -ENOMEM;
		goto err;
	}

	val = *(uint32_t *)(mmio + SLOT_NUM_REG);
	/* Read Slot-Num Register */
	printk("[VA: %#lx]Broiler Slot Num: %d\n", mmio, *(uint32_t *)(mmio + SLOT_NUM_REG));
	/* Setup Slot-Sel Register */
	*(uint32_t *)(mmio + SLOT_SEL_REG) = 0x02;
	/* Frequency */
	printk("Broiler Slot-Sel: %d\n  Frequency Range [%#x - %#x]\n",
				*(uint32_t *)(mmio + SLOT_SEL_REG),
				*(uint32_t *)(mmio + MIN_FREQ_REG),
				*(uint32_t *)(mmio + MAX_FREQ_REG));

	return 0;

err:
	remove_resource(&Broiler_pio_res);
	return r;
}

static void __exit BiscuitOS_exit(void)
{
	iounmap(mmio);
	remove_resource(&Broiler_pio_res);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Broiler Synchronous MMIO In-Kernel on BiscuitOS");
