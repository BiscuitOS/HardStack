/*
 * Input Touch Device Driver
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
 *        input_touch {
 *                compatible = "BiscuitOS, input_touch";
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
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/random.h>

/* LDD Platform Name */
#define DEV_NAME	"input_touch"

#define X_AXIS_MIN	0
#define X_AXIS_MAX	1023
#define Y_AXIS_MAX	X_AXIS_MAX
#define Y_AXIS_MIN	X_AXIS_MIN
#define PRESSURE_MAX	X_AXIS_MAX
#define PRESSURE_MIN	X_AXIS_MIN

/* Private data */
struct input_touch_pdata {
	int irq;
	int gpio;
	struct input_dev *input;
};

static irqreturn_t input_touch_isr(int irq, void *dev_id)
{
	struct input_touch_pdata *pdata = (struct input_touch_pdata *)dev_id;
	int value = gpio_get_value(pdata->gpio);
	/* Emulate input X/Y/Press value */
	int emulate_x = get_random_u32() % X_AXIS_MAX;
	int emulate_y = get_random_u32() % Y_AXIS_MAX;

	if (value) {
		/* Press touch */
		input_report_abs(pdata->input, ABS_X, emulate_x);
		input_report_abs(pdata->input, ABS_Y, emulate_y);
		input_report_abs(pdata->input, ABS_PRESSURE, 1);
	} else {
		/* Up touch */
		input_report_abs(pdata->input, ABS_PRESSURE, 0);
		input_report_key(pdata->input, BTN_TOUCH, 0);
	}

	/* sync report */
	input_sync(pdata->input);

	return IRQ_HANDLED;
}

/* input event open */
static int input_touch_open(struct input_dev *input)
{
	struct input_touch_pdata *pdata = input_get_drvdata(input);

	printk("Event open.. Touch Screen: %d\n", pdata->gpio);

	return 0;
}

/* input event close */
static void input_touch_close(struct input_dev *input)
{
	struct input_touch_pdata *pdata = input_get_drvdata(input);

	printk("Event close.. Touch Screen: %d\n", pdata->gpio);
}

/* Probe: (LDD) Initialize Device */
static int input_touch_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct input_touch_pdata *pdata;
	struct input_dev *input;
	int ret;

	/* Allocate Memory */
	pdata = (struct input_touch_pdata *)kzalloc(sizeof(*pdata), 
								GFP_KERNEL);
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

	/* Setup Touch Event */
	input->evbit[0] = BIT_MASK(EV_SYN) | 
			  BIT_MASK(EV_KEY) | 
			  BIT_MASK(EV_ABS);
	/* Setup Touch type */
	input->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	input_set_abs_params(input, ABS_X, X_AXIS_MIN, X_AXIS_MAX, 0, 0);
	input_set_abs_params(input, ABS_Y, Y_AXIS_MIN, Y_AXIS_MAX, 0, 0);
	input_set_abs_params(input, ABS_PRESSURE, PRESSURE_MIN, 
							PRESSURE_MAX, 0, 0);

	/* input information */
	input->name		= DEV_NAME;
	input->phys		= DEV_NAME "/input0";
	input->dev.parent	= &pdev->dev;
	input->id.bustype	= BUS_HOST;
	input->id.vendor	= 0x9192;
	input->id.product	= 0x1016;
	input->id.version	= 0x1413;

	/* Create /dev/input/eventx interface */
	input->open  = input_touch_open;
	input->close = input_touch_close;
	/* Note! must setup before input_register_device() */
	input_set_drvdata(input, pdata);

	/* Register input device */
	ret = input_register_device(input);
	if (ret) {
		dev_err(&pdev->dev, "Unable to register input device\n");
		goto err_input_register;
	}

	/* Register Interrupt: Press and Up trigger interrupt */
	ret = request_irq(pdata->irq, input_touch_isr, 
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
static int input_touch_remove(struct platform_device *pdev)
{
	struct input_touch_pdata *pdata = platform_get_drvdata(pdev);

	free_irq(pdata->irq, (void *)pdata);
	input_unregister_device(pdata->input);
	input_free_device(pdata->input);
	kfree(pdata);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id input_touch_of_match[] = {
	{ .compatible = "BiscuitOS, input_touch", },
	{ },
};
MODULE_DEVICE_TABLE(of, input_touch_of_match);

/* Platform Driver Information */
static struct platform_driver input_touch_driver = {
	.probe    = input_touch_probe,
	.remove   = input_touch_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
		.of_match_table	= input_touch_of_match,
	},
};
module_platform_driver(input_touch_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Input Touch Device Driver with DTS");
