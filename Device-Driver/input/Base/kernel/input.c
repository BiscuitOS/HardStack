/*
 * Input Device Driver
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

/* DDL Input Name */
#define DEV_NAME "Input_demo"
/* Input device */
static struct input_dev *Input_demo;
/* Timer: Emulate Input Source */
static struct timer_list Timer;
/* Input Source Period */
#define INPUT_PERIOD	1000 /* 1000ms -> 1s */

/* Private data */
struct Input_demo_pdata {
	int index;
	struct input_dev *input;
};

/* Emulate key value */
extern u32 get_random_u32(void);
/* Timer handler: Emulate Input source */
static void Timer_handler(struct timer_list *unused)
{
	int emulate_key_value = get_random_u32() % 2;
	u32 event = 0;

	/* Repore event */
	input_event(Input_demo, EV_KEY, event, emulate_key_value);
	input_sync(Input_demo);
	/* Timer: Setup Timeout */
	Timer.expires = jiffies + msecs_to_jiffies(INPUT_PERIOD);
	/* Timer: Register */
	add_timer(&Timer);
}

/* input event open */
static int Input_demo_open(struct input_dev *input)
{
	input_sync(input);
	return 0;
}

/* input event close */
static void Input_demo_close(struct input_dev *input)
{
}

/* Probe: (DDL) Initialize Device */
static int Input_demo_probe(struct platform_device *pdev)
{
	struct Input_demo_pdata *pdata;
	struct input_dev *input;
	int rvl;

	/* Build private data */
	pdata = (struct Input_demo_pdata *)kzalloc(sizeof(*pdata), 
							GFP_KERNEL);
	if (!pdata) {
		rvl = -ENOMEM;
		goto err_pdata;
	}

	/* Create Input Device
	 * --> devm_input_allocate_device()
	 * --> input_allocate_device()
	 */
	input = devm_input_allocate_device(&pdev->dev);
	if (!input) {
		dev_err(&pdev->dev, "Unable allocate memory to input dev\n");
		rvl = -ENOMEM;
		goto err_input_alloc;
	}

	/* Setup Driver data */
	pdata->index = 0x91;
	pdata->input = input;
	Input_demo = input; /* In order to emulate key press */
	input_set_drvdata(input, pdata);
	platform_set_drvdata(pdev, pdata);

	/* Setup Input device */
	input->name = DEV_NAME;
	input->dev.parent = &pdev->dev;
	input->open  = Input_demo_open;
	input->close = Input_demo_close;

	input->id.bustype = BUS_HOST;
	input->id.vendor  = 0x9192;
	input->id.product = 0x1016;
	input->id.version = 0x1413;

	rvl = input_register_device(input);
	if (rvl) {
		dev_err(&pdev->dev, "Unable to register input device\n");
		goto err_input_register;
	}

	/* Add a Timer to emulate input source */
	timer_setup(&Timer, Timer_handler, 0);
	/* Input source period */
	Timer.expires = jiffies + msecs_to_jiffies(INPUT_PERIOD);
	/* Timing begin */
	add_timer(&Timer);

	return 0;

err_pdata:

err_input_alloc:
	kfree(pdata);
err_input_register:
	input_free_device(input);

	return rvl;
}

/* Remove: (DDL) Remove Device (Module) */
static int Input_demo_remove(struct platform_device *pdev)
{
	struct Input_demo_pdata *pdata = platform_get_drvdata(pdev);
	struct input_dev *input = pdata->input;

	/* Stop Timer */
	del_timer(&Timer);
	/* unregister device */
	input_unregister_device(input);
	input_free_device(input);
	kfree(pdata);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

/* Shutdown: (DDL) Power-off/Shutdown */
static void Input_demo_shutdown(struct platform_device *pdev)
{
}

/* Suspend: (DDL) Suspend (schedule) Sleep */
static int Input_demo_suspend(struct platform_device *pdev, 
							pm_message_t state)
{
	return 0;
}

/* Resume: (DDL) (schedule) From Suspend/Sleep */
static int Input_demo_resume(struct platform_device *pdev)
{
	return 0;
}

/* Input Device Release */
static void Input_demo_dev_release(struct device *dev)
{
	dev->parent = NULL;
}

/* Input Driver Information */
static struct platform_driver Input_demo_driver = {
	.probe    = Input_demo_probe,
	.remove   = Input_demo_remove,
	.shutdown = Input_demo_shutdown,
	.suspend  = Input_demo_suspend,
	.resume   = Input_demo_resume,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
	},
};

/* Input Device Information */
static struct platform_device Input_demo_device = {
	.name = DEV_NAME,
	.id = 1,
	.dev = {
		.release = Input_demo_dev_release,
	}
};

/* Module initialize entry */
static int __init Input_demo_init(void)
{
	int ret;

	/* Register platform driver */
	ret = platform_driver_register(&Input_demo_driver);
	if (ret) {
		printk("Unable register Input driver.\n");
		return -EBUSY;
	}

	/* Register platform device */
	ret = platform_device_register(&Input_demo_device);
	if (ret) {
		printk("Unable register Input device.\n");
		return -EBUSY;
	}

	return 0;
}

/* Module exit entry */
static void __exit Input_demo_exit(void)
{
	platform_device_unregister(&Input_demo_device);
	platform_driver_unregister(&Input_demo_driver);
}

module_init(Input_demo_init);
module_exit(Input_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Input Device Driver");
