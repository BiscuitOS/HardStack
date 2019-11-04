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
 * DTS file: arch/arm/boot/dts/bcm2711-rpi-4-b.dts 
 *
 * &soc {
 *        UIO_intr {
 *                compatible = "BiscuitOS, UIO_intr";
 *                status = "okay";
 *                BD-gpio = <&gpio 25 GPIO_ACTIVE_HIGH>;
 *        };
 * };
 */

/* PaspberryPi 4B GPIO
 *
 *                    +---------+
 *                3V3 | 1     2 | 5V
 *       (SDA1) GPIO2 | 3     4 | 5V
 *       (SCL1) GPIO3 | 5     6 | GND
 *  (GPIO_GCLK) GPIO4 | 7     8 | GPIO14 (TXD0)
 *                GND | 9    10 | GPIO15 (RXD0)
 * (GPIO_GEN0) GPIO17 | 11   12 | GPIO18 (GPIO_GEN1)
 * (GPIO_GEN2) GPIO27 | 13   14 | GND
 * (GPIO_GEN3) GPIO22 | 15   16 | GPIO23 (GPIO_GEN4)
 *               3V3  | 17   18 | GPIO24 (GPIO_GEN5)
 *  (SPI_MOSI) GPIO10 | 19   20 | GND
 *   (SPI_MISO) GPIO9 | 21   22 | GPIO25 (GPIO_GEN6) <------ Interrupt
 *  (SPI_SCLK) GPIO11 | 23   24 | GPIO8  (SPI_CE0_N)
 *                GND | 25   26 | GPIO7  (SPI_CE1_N)
 *              ID_SD | 27   28 | ID_SC
 *              GPIO5 | 29   30 | GND
 *              GPIO6 | 31   32 | GPIO12
 *             GPIO13 | 33   34 | GND
 *             GPIO19 | 35   36 | GPIO16
 *             GPIO26 | 37   38 | GPIO20
 *                GND | 39   40 | GPIO21
 *                    +---------+
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/uio_driver.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>

/* LDD Platform Name */
#define DEV_NAME	"UIO_intr"
#define UIO_VERSION	"0.11"

/* 0 - irq disable  1 - irq enable */
static int irq_state = 0;

/* IRQ handler */
static irqreturn_t UIO_intr_isr(int irq, struct uio_info *info)
{
	/* Disable irq and enable on userspace */
	printk("Interrupt on kernel\n");
	return IRQ_RETVAL(IRQ_HANDLED);
}

/* IRQ controller */
static int UIO_intr_irqcontrol(struct uio_info *info, s32 irq_on)
{
	if (irq_on && !irq_state) {
		enable_irq(info->irq);
		irq_state = 1;
	} else if (!irq_on && irq_state) {
		disable_irq_nosync(info->irq);
		irq_state = 0;
	}

	return 0;
}

/* Probe: (LDD) Initialize Device */
static int UIO_intr_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct uio_info *uinfo;
	int ret, gpio;

	/* Allocate Memory */
	uinfo = kzalloc(sizeof(*uinfo), GFP_KERNEL);
	if (!uinfo) {
		printk("ERROR: No free memory\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	/* Get IRQ number from DTB */
	gpio = of_get_named_gpio(np, "BD-gpio", 0);
	if (gpio < 0) {
		printk("Unable to get gpio from DTS\n");
		ret = -EINVAL;
		goto err_free;
	}
	gpio_direction_input(gpio);
	uinfo->irq = gpio_to_irq(gpio);
	if (uinfo->irq < 0) {
		printk("ERROR: Unable to get IRQ\n");
		ret = -EINVAL;
		goto err_free;
	}

	/* setup UIO */
	uinfo->name       = DEV_NAME;
	uinfo->version    = UIO_VERSION;
	uinfo->irq_flags  = IRQF_TRIGGER_FALLING | IRQF_SHARED;
	uinfo->handler    = UIO_intr_isr;
	uinfo->irqcontrol = UIO_intr_irqcontrol;

	/* Register UIO device */
	ret = uio_register_device(&pdev->dev, uinfo);
	if (ret) {
		printk("ERROR: Register UIO\n");
		ret = -ENODEV;
		goto err_free;
	}

	disable_irq_nosync(uinfo->irq);
	irq_state = 0;
	platform_set_drvdata(pdev, &uinfo);
	printk("UIO Interrupt %ld Register OK...\n", uinfo->irq);

	return 0;

err_free:
	kfree(uinfo);
err_alloc:
	return ret;
}

/* Remove: (LDD) Remove Device (Module) */
static int UIO_intr_remove(struct platform_device *pdev)
{
	struct uio_info *uinfo = platform_get_drvdata(pdev);

	if (irq_state)
		disable_irq_nosync(uinfo->irq);
	uio_unregister_device(uinfo);

	kfree(uinfo);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id UIO_intr_of_match[] = {
	{ .compatible = "BiscuitOS, UIO_intr", },
	{ },
};
MODULE_DEVICE_TABLE(of, UIO_intr_of_match);

/* Platform Driver Information */
static struct platform_driver UIO_intr_driver = {
	.probe    = UIO_intr_probe,
	.remove   = UIO_intr_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
		.of_match_table	= UIO_intr_of_match,
	},
};
module_platform_driver(UIO_intr_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("UIO Interrupt Device Driver with DTS");
