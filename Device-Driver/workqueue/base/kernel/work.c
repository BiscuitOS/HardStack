/*
 * Work_queue Device Driver
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
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>

/* LDD Platform Name */
#define DEV_NAME "WQ_demo"

#define INPUT_PERIOD		1000

struct WQ_demo_pdata
{
	struct work_struct wq;
	struct timer_list timer;
};

/* work queue handler */
void wq_isr(struct work_struct *work)
{
	printk("Work....\n");
}

/* Timer interrupt handler */
static void timer_isr(struct timer_list *unused)
{
	struct WQ_demo_pdata *pdata;

	pdata = container_of(unused, struct WQ_demo_pdata, timer);

	/* wakeup interrupt bottom */
	schedule_work(&pdata->wq);

	/* Timer: setup timeout */
	pdata->timer.expires = jiffies + msecs_to_jiffies(INPUT_PERIOD);
	/* Timer: Register */
	add_timer(&pdata->timer);
}

/* Probe: (LDD) Initialize Device */
static int WQ_demo_probe(struct platform_device *pdev)
{
	struct WQ_demo_pdata *pdata;
	int ret;

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		printk("Error: alloc pdata\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	/* Initialize wq */
	INIT_WORK(&pdata->wq, wq_isr);

	/* Emulate interrupt via timer */
	timer_setup(&pdata->timer, timer_isr, 0);
	pdata->timer.expires = jiffies + msecs_to_jiffies(100);
	add_timer(&pdata->timer);

	platform_set_drvdata(pdev, pdata);

	return 0;

err_alloc:
	return ret;
}

/* Remove: (LDD) Remove Device (Module) */
static int WQ_demo_remove(struct platform_device *pdev)
{
	struct WQ_demo_pdata *pdata = platform_get_drvdata(pdev);
	
	del_timer(&pdata->timer);
	kfree(pdata);

	return 0;
}

/* Platform Driver Information */
static struct platform_driver WQ_demo_driver = {
	.probe    = WQ_demo_probe,
	.remove   = WQ_demo_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
	},
};

static struct platform_device *pdev;
/* Module initialize entry */
static int __init WQ_demo_init(void)
{
	int ret;

	ret = platform_driver_register(&WQ_demo_driver);
	if (ret) {
		printk("Error: Platform driver register.\n");
		return -EBUSY;
	}

	pdev = platform_device_register_simple(DEV_NAME, 1, NULL, 0);
	if (IS_ERR(pdev)) {
		printk("Error: Platform device register\n");
		return PTR_ERR(pdev);
	}
	return 0;
}

/* Module exit entry */
static void __exit WQ_demo_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&WQ_demo_driver);
}

module_init(WQ_demo_init);
module_exit(WQ_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Work_queue Device Driver");
