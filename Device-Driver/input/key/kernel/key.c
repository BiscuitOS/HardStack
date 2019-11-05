/*
 * Input Key Device Driver
 *
 * (C) 2019.11.04 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * DTS file: arch/arm/boot/dts/bcm2711-rpi-4-b.dts 
 *
 * &soc {
 *        input_key {
 *                compatible = "BiscuitOS, input_key";
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
 *   (SPI_MISO) GPIO9 | 21   22 | GPIO25 (GPIO_GEN6) <------ Interrupt/Key
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
 *
 *
 * Key
 *
 *                       | A
 *                 Press | | Up
 *                       V |
 *                    +-------+
 *                    |  Key  | 
 *                +---------------+
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/input.h>

/* LDD Platform Name */
#define DEV_NAME	"input_key"

/* Private data */
struct input_key_pdata {
	int irq;
	int gpio;
	struct input_dev *input;
};

static irqreturn_t input_key_isr(int irq, void *dev_id)
{
	struct input_key_pdata *pdata = (struct input_key_pdata *)dev_id;
	int value = gpio_get_value(pdata->gpio);

	if (value) {
		printk("Event: KEY_DOWN --> UP\n");

		/* arg0 - input device
		 * arg1 - Key type
		 * arg2 - Key state [1 DOWN:0 UP]
		 */
		input_report_key(pdata->input, KEY_DOWN, 0);
	} else {
		printk("Event: KEY_DOWN --> Press\n");

		/* arg0 - input device
		 * arg1 - Key type
		 * arg2 - Key state [1 DOWN:0 UP]
		 */
		input_report_key(pdata->input, KEY_DOWN, 1);
	}
	/* sync report */
	input_sync(pdata->input);

	return IRQ_HANDLED;
}

/* input event open */
static int input_key_open(struct input_dev *input)
{
	struct input_key_pdata *pdata = input_get_drvdata(input);

	printk("Event open.. KEY-GPIO: %d\n", pdata->gpio);

	return 0;
}

/* input event close */
static void input_key_close(struct input_dev *input)
{
	struct input_key_pdata *pdata = input_get_drvdata(input);

	printk("Event close.. KEY-GPIO: %d\n", pdata->gpio);
}

/* Probe: (LDD) Initialize Device */
static int input_key_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct input_key_pdata *pdata;
	struct input_dev *input;
	int ret;

	/* Allocate Memory */
	pdata = (struct input_key_pdata *)kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		printk("ERROR: No free memory\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	/* Get IRQ number from DTB */
	pdata->gpio = of_get_named_gpio(np, "BD-gpio", 0);
	if (pdata->gpio < 0) {
		printk("Unable to get gpio from DTS\n");
		ret = -EINVAL;
		goto err_free;
	}
	gpio_direction_input(pdata->gpio);
	pdata->irq = gpio_to_irq(pdata->gpio);
	if (pdata->irq < 0) {
		printk("ERROR: Unable to get IRQ\n");
		ret = -EINVAL;
		goto err_free;
	}

	/* Create Input device
	 * --> input_allocate_device()
	 * --> input_allocate_device()
	 */
	input = devm_input_allocate_device(&pdev->dev);
	if (!input) {
		printk("Error: Unable to alloc input device.\n");
		ret = -ENOMEM;
		goto err_free;
	}

	/* Setup Key Event */
	input->evbit[BIT_WORD(EV_KEY)] |= BIT_MASK(EV_KEY);
	/* Setup key type */
	input->keybit[BIT_WORD(KEY_UP)]		|= BIT_MASK(KEY_UP);
	input->keybit[BIT_WORD(KEY_DOWN)]	|= BIT_MASK(KEY_DOWN);
	input->keybit[BIT_WORD(KEY_LEFT)]	|= BIT_MASK(KEY_LEFT);
	input->keybit[BIT_WORD(KEY_RIGHT)]	|= BIT_MASK(KEY_RIGHT);

	/* input information */
	input->name		= DEV_NAME;
	input->phys		= DEV_NAME "/input0";
	input->dev.parent	= &pdev->dev;
	input->id.bustype	= BUS_HOST;
	input->id.vendor	= 0x9192;
	input->id.product	= 0x1016;
	input->id.version	= 0x1413;

	/* Create /dev/input/eventx interface */
	input->open  = input_key_open;
	input->close = input_key_close;
	/* Note! must setup before input_register_device() */
	input_set_drvdata(input, pdata);

	/* Register input device */
	ret = input_register_device(input);
	if (ret) {
		dev_err(&pdev->dev, "Unable to register input device\n");
		goto err_input_register;
	}

	/* Register Interrupt: Press and Up trigger interrupt */
	ret = request_irq(pdata->irq, input_key_isr, 
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, 
						DEV_NAME, (void *)pdata);
	if (ret < 0) {
		printk("Error: Request IRQ %d failed\n", pdata->irq);
		goto err_irq;
	}

	pdata->input = input;
	platform_set_drvdata(pdev, pdata);
	return 0;

err_irq:
	input_unregister_device(input);
err_input_register:
	input_free_device(input);
err_free:
	kfree(pdata);
err_alloc:
	return ret;
}

/* Remove: (LDD) Remove Device (Module) */
static int input_key_remove(struct platform_device *pdev)
{
	struct input_key_pdata *pdata = platform_get_drvdata(pdev);

	free_irq(pdata->irq, (void *)pdata);
	input_unregister_device(pdata->input);
	input_free_device(pdata->input);
	kfree(pdata);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id input_key_of_match[] = {
	{ .compatible = "BiscuitOS, input_key", },
	{ },
};
MODULE_DEVICE_TABLE(of, input_key_of_match);

/* Platform Driver Information */
static struct platform_driver input_key_driver = {
	.probe    = input_key_probe,
	.remove   = input_key_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
		.of_match_table	= input_key_of_match,
	},
};
module_platform_driver(input_key_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Input Key Device Driver with DTS");
