/*
 * Broiler Asynchronous MMIO on BiscuitOS
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
#include <linux/interrupt.h>

#define BROILER_MMIO_BASE	0xD0000040
#define BROILER_MMIO_LEN	0x10
#define BOORBALL_REG		0x00
#define IRQ_NUM_REG		0x04

static struct resource Broiler_mmio_res = {
	.name	= "Broiler Asynchronous MMIO",
	.start	= BROILER_MMIO_BASE,
	.end	= BROILER_MMIO_BASE + BROILER_MMIO_LEN,
	.flags	= IORESOURCE_MEM,
};
static void __iomem *mmio;

static irqreturn_t Broiler_irq_handler(int irq, void *vdev)
{
	/* TODO */
	printk("Broiler Asynchronous MMIO with IRQ %d\n", irq);
	return IRQ_HANDLED;
}

static int __init BiscuitOS_init(void)
{
	int r, irq;

	r = request_resource(&iomem_resource, &Broiler_mmio_res);
	if (r < 0)
		return r;

	mmio = ioremap(BROILER_MMIO_BASE, BROILER_MMIO_LEN);
	if (!mmio) {
		r = -ENOMEM;
		goto err;
	}

	/* Irq number */
	irq = *(uint32_t *)(mmio + IRQ_NUM_REG);
	/* Register IRQ */
	r = request_irq(irq, Broiler_irq_handler,
				IRQF_TRIGGER_HIGH,
				"Broiler-Asynchronous-MMIO-vPIC", NULL);
	if (r < 0) {
		printk("ERROR: Request IRQ failed.\n");
		goto err_irq;
	}

	printk("Asynchronous MMIO Register Success.\n");

	/* Trigger Asynchronous and Raise Interrupt */
	*(uint32_t *)(mmio + BOORBALL_REG) = 0x01;

	return 0;

err_irq:
	iounmap(mmio);
err:
	remove_resource(&Broiler_mmio_res);
	return r;
}

static void __exit BiscuitOS_exit(void)
{
	iounmap(mmio);
	remove_resource(&Broiler_mmio_res);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Broiler Asynchronous MMIO on BiscuitOS");
