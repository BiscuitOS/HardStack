/*
 * GPIO Device Driver
 *
 * (C) 2019.10.24 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 *   (SPI_MISO) GPIO9 | 21   22 | GPIO25 (GPIO_GEN6)
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

/*
 * Private DTS file: DTS_demo.dtsi
 *
 * / {
 *        GPIO_demo {
 *                compatible = "BiscuitOS, GPIO";
 *                status = "okay";
 *                BD-gpio = <&gpio 27 GPIO_ACTIVE_HIGH>;
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
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>

/* DDL Platform Name */
#define DEV_NAME "GPIO_demo"

struct GPIO_demo_priv
{
	int irq;
	int gpio;
};

/* IRQ handler */
static irqreturn_t GPIO_demo_handler(int irq, void *dev_id)
{
	printk("IRQ %d handler\n", irq);
	return IRQ_HANDLED;
}

/* Probe: (LDD) Initialize Device */
static int GPIO_demo_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct GPIO_demo_priv *priv;
	int value, gpio, irq;
	int ret;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		printk("ERROR: No free memory\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	gpio = of_get_named_gpio(np, "BD-gpio", 0);
	if (gpio < 0) {
		printk("Unable to get gpio from DTS\n");
		ret = -EINVAL;
		goto err_gpio;
	}

	/* Setup gpio direction
	 *
	 * --> input
	 *     --> gpio_direction_input()
	 * --> output
	 *     --> gpio_direction_output()
	 */
	gpio_direction_output(gpio, 0);

	/* Setup gpio value */
	gpio_set_value(gpio, 1);
	/* Get gpio value */
	value = gpio_get_value(gpio);
	printk("GPIO-%d: %d\n", gpio, value);

	/* Interrupt */
	gpio_direction_input(gpio);

	/* IRQ number from GPIO */
	irq = gpio_to_irq(gpio);

	/* Request IRQ */
	ret = request_irq(irq, GPIO_demo_handler, 
				IRQF_TRIGGER_FALLING, DEV_NAME, NULL);
	if (ret < 0) {
		printk("Can't request IRQ %d\n", irq);
		ret = -EINVAL;
		goto err_irq;
	}
	printk("GPIO-%d IRQ: %d\n", gpio, irq);

	/* setup private data */
	priv->irq = irq;
	priv->gpio = gpio;
	platform_set_drvdata(pdev, priv);

	return 0;

err_irq:
err_gpio:
	kfree(priv);
err_alloc:
	return ret;
}

/* Remove: (LDD) Remove Device (Module) */
static int GPIO_demo_remove(struct platform_device *pdev)
{
	struct GPIO_demo_priv *priv = platform_get_drvdata(pdev);

	/* Free IRQ */
	free_irq(priv->irq, NULL);

	kfree(priv);
	return 0;
}

static const struct of_device_id GPIO_demo_of_match[] = {
	{ .compatible = "BiscuitOS, GPIO", },
	{ },
};
MODULE_DEVICE_TABLE(of, GPIO_demo_of_match);

/* Platform Driver Information */
static struct platform_driver GPIO_demo_driver = {
	.probe    = GPIO_demo_probe,
	.remove   = GPIO_demo_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
		.of_match_table	= GPIO_demo_of_match,
	},
};
module_platform_driver(GPIO_demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("GPIO Device Driver with DTS");
