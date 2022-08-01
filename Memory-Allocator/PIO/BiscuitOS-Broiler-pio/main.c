/*
 * Broiler PIO on BiscuitOS
 *
 * (C) 2022.08.01 BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define BISCUITOS_PIO_PORT	0x6800
#define BISCUITOS_PIO_LEN	0x10
#define SLOT_NUM_REG		0x00
#define SLOT_SEL_REG		0x04
#define MIN_FREQ_REG		0x08
#define MAX_FREQ_REG		0x0C

static struct resource Broiler_pio_res = {
	.name	= "Broiler PIO",
	.start	= BISCUITOS_PIO_PORT,
	.end	= BISCUITOS_PIO_PORT + BISCUITOS_PIO_LEN,
	.flags	= IORESOURCE_IO,
};
static void __iomem *iomem;

static int __init BiscuitOS_init(void)
{
	int r;

	r = request_resource(&ioport_resource, &Broiler_pio_res);
	if (r < 0)
		return r;

	iomem = ioport_map(BISCUITOS_PIO_PORT, BISCUITOS_PIO_LEN);
	if (!iomem) {
		r = -ENOMEM;
		goto err;
	}

	/* Read Slot-Num Register */
	printk("Broiler Slot Num: %d\n", ioread32(iomem + SLOT_NUM_REG));
	/* Setup Slot-Sel Register */
	iowrite32(0x02, iomem + SLOT_SEL_REG);
	/* Frequency */
	printk("Broiler Slot-Sel: %d\n  Frequency Range [%#x - %#x]\n",
				ioread32(iomem + SLOT_SEL_REG),
				ioread32(iomem + MIN_FREQ_REG),
				ioread32(iomem + MAX_FREQ_REG));

	return 0;

err:
	remove_resource(&Broiler_pio_res);
	return r;
}

static void __exit BiscuitOS_exit(void)
{
	ioport_unmap(iomem);
	remove_resource(&Broiler_pio_res);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Broiler PIO on BiscuitOS");
