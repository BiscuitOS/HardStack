/*
 * Broiler Interrupt with vIOAPIC
 *
 * (C) 2022.08.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/io.h>

#define BISCUITOS_PIO_PORT	0x6040
#define BISCUITOS_PIO_LEN	0x10
#define IRQ_NUM_REG		0x04

static struct resource Broiler_pio_res = {
	.name	= "Broiler PIO vIOAPIC",
	.start	= BISCUITOS_PIO_PORT,
	.end	= BISCUITOS_PIO_PORT + BISCUITOS_PIO_LEN,
	.flags	= IORESOURCE_IO,
};
static void __iomem *iomem;
static int irq;

static irqreturn_t Broiler_irq_handler(int irq, void *vdev)
{
	/* TODO */
	printk("Broiler vIOAPIC with IRQ %d\n", irq);
	return IRQ_HANDLED;
}

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

	irq = ioread32(iomem + IRQ_NUM_REG);
	r = request_irq(irq, Broiler_irq_handler,
			IRQF_TRIGGER_RISING, "Broiler-PIO-vIOAPIC", NULL);
	if (r < 0) {
		printk("ERROR: Request IRQ failed.\n");
		goto err_irq;
	}

	printk("I/O Port register success.\n");

	/* Trigger Interrupt */
	iowrite16(0x01, iomem);

	return 0;

err_irq:
	ioport_unmap(iomem);
err:
	remove_resource(&Broiler_pio_res);
	return r;
}

static void __exit BiscuitOS_exit(void)
{
	free_irq(irq, NULL);
	ioport_unmap(iomem);
	remove_resource(&Broiler_pio_res);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Broiler PIO with vIOAPIC");
