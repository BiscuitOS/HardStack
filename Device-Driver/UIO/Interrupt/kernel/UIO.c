/*
 * UIO Interrupt Device Driver
 *
 * (C) 2019.10.30 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Private DTS file: DTS_demo.dtsi
 *
 * / {
 *        UIO_demo {
 *                compatible = "BiscuitOS, UIO";
 *                status = "okay";
 *                interrupts-parent = <&gpio>;
 *                interrupts = <&gpio 58 IRQ_TYPE_LEVEL_HIGH>;
 *        };
 * };
 *
 * On Core dtsi:
 *
 * include "DTS_demo.dtsi"
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/uio_driver.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>

/* LDD Platform Name */
#define DEV_NAME	"UIO_demo"
#define UIO_VERSION	"0.11"

/* IRQ handler */
static irqreturn_t UIO_demo_isr(int irq, struct uio_info *info)
{
	disable_irq_nosync(info->irq);
	uio_event_notify(info);
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

/* IRQ controller */
static int UIO_demo_irqcontrol(struct uio_info *info, s32 irq_on)
{
	if (irq_on)
		enable_irq(info->irq);
	else
		disable_irq(info->irq);

	return 0;
}

/* Probe: (LDD) Initialize Device */
static int UIO_demo_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct uio_info *uinfo;
	int ret;

	/* Allocate Memory */
	uinfo = kzalloc(sizeof(*uinfo), GFP_KERNEL);
	if (!uinfo) {
		printk("ERROR: No free memory\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	/* Get IRQ number from DTB */
	uinfo->irq = of_irq_get(np, 0);
	if (uinfo->irq < 0) {
		printk("ERROR: Unable to get IRQ\n");
		ret = -EINVAL;
		goto err_irq;
	}
	/* setup UIO */
	uinfo->name       = DEV_NAME;
	uinfo->version    = UIO_VERSION;
	uinfo->irq_flags  = IRQF_TRIGGER_FALLING;
	uinfo->handler    = UIO_demo_isr;
	uinfo->irqcontrol = UIO_demo_irqcontrol;

	/* Register UIO device */
	ret = uio_register_device(&pdev->dev, uinfo);
	if (ret) {
		printk("ERROR: Register UIO\n");
		ret = -ENODEV;
		goto err_uio;
	}

	platform_set_drvdata(pdev, &uinfo);
	printk("UIO Interrupt Register OK...\n");

	return 0;

err_uio:
err_irq:
	kfree(uinfo);
err_alloc:

	return ret;
}

/* Remove: (LDD) Remove Device (Module) */
static int UIO_demo_remove(struct platform_device *pdev)
{
	struct uio_info *uinfo = platform_get_drvdata(pdev);

	uio_unregister_device(uinfo);

	kfree(uinfo);
	return 0;
}

static const struct of_device_id UIO_demo_of_match[] = {
	{ .compatible = "BiscuitOS, UIO_demo", },
	{ },
};
MODULE_DEVICE_TABLE(of, UIO_demo_of_match);

/* Platform Driver Information */
static struct platform_driver UIO_demo_driver = {
	.probe    = UIO_demo_probe,
	.remove   = UIO_demo_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
		.of_match_table	= UIO_demo_of_match,
	},
};
module_platform_driver(UIO_demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("UIO Interrupt Device Driver with DTS");
