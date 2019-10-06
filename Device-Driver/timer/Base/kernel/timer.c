/*
 * Timer Device Driver
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
#include <linux/timer.h>
#include <linux/platform_device.h>

/* DDL: Timer Name */
#define DEV_NAME "Timer_demo"

/* Timer */
static struct timer_list Timer;
/* Timer Period */
#define TIMER_DEMO_PERIOD	1000 /* 1000ms -> 1s */

/* Timer: handler Timerout */
static void Timer_demo_handler(struct timer_list *unused)
{
	printk("BiscuitOS Timer Timeout\n");
	/* Timer: Setup Timeout */
	Timer.expires = jiffies + msecs_to_jiffies(TIMER_DEMO_PERIOD);
	/* Timer: Register */
	add_timer(&Timer);
}

/* Probe: (DDL) Initialize Device */
static int Timer_demo_probe(struct platform_device *pdev)
{
	/* Timer: Setup Timer */
	timer_setup(&Timer, Timer_demo_handler, 0);
	/* Timer: Setup Timeout */
	Timer.expires = jiffies + msecs_to_jiffies(TIMER_DEMO_PERIOD);
	/* Timer: Register */
	add_timer(&Timer);

	return 0;
}

/* Remove: (DDL) Remove Device (Module) */
static int Timer_demo_remove(struct platform_device *pdev)
{
	/* Timer: Unregister */
	del_timer(&Timer);

	return 0;
}

/* Shutdown: (DDL) Power-off/Shutdown */
static void Timer_demo_shutdown(struct platform_device *pdev)
{
}

/* Suspend: (DDL) Suspend (schedule) Sleep */
static int Timer_demo_suspend(struct platform_device *pdev, 
							pm_message_t state)
{
	return 0;
}

/* Resume: (DDL) (schedule) From Suspend/Sleep */
static int Timer_demo_resume(struct platform_device *pdev)
{
	return 0;
}

/* Input Device Release */
static void Timer_demo_dev_release(struct device *dev)
{
	dev->parent = NULL;
}

/* Input Driver Information */
static struct platform_driver Timer_demo_driver = {
	.probe    = Timer_demo_probe,
	.remove   = Timer_demo_remove,
	.shutdown = Timer_demo_shutdown,
	.suspend  = Timer_demo_suspend,
	.resume   = Timer_demo_resume,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
	},
};

/* Input Device Information */
static struct platform_device Timer_demo_device = {
	.name = DEV_NAME,
	.id = 1,
	.dev = {
		.release = Timer_demo_dev_release,
	}
};

/* Module initialize entry */
static int __init Timer_demo_init(void)
{
	int ret;

	/* Register platform driver */
	ret = platform_driver_register(&Timer_demo_driver);
	if (ret) {
		printk("Unable register Input driver.\n");
		return -EBUSY;
	}

	/* Register platform device */
	ret = platform_device_register(&Timer_demo_device);
	if (ret) {
		printk("Unable register Input device.\n");
		return -EBUSY;
	}

	return 0;
}

/* Module exit entry */
static void __exit Timer_demo_exit(void)
{
	platform_device_unregister(&Timer_demo_device);
	platform_driver_unregister(&Timer_demo_driver);
}

module_init(Timer_demo_init);
module_exit(Timer_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Timer Device Driver");
