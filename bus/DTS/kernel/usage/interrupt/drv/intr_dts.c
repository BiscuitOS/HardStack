/*
 * Interrupt on DTS
 *
 * (C) 2018.11.14 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 *        demo: demo2@2 {
 *                compatible = "demo,demo_intr";
 *                reg = <2>;
 *                interrupt-parent = <&gpio0>;
 *                interrupts = <11 2>;
 *                status = "okay";
 *        };
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

/* define name for device and driver */
#define DEV_NAME "demo_intr"
static int irq;

static irqreturn_t demo_irq_handler(int irq, void *dev_id)
{

    printk("Hello World\n");

    return IRQ_HANDLED;
}

/* probe platform driver */
static int demo_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    int ret;
    
    /* Obtain interrupt ID from DTS */
    irq = of_irq_get(np, 0);

    /* Request irq handler */
    ret = request_threaded_irq(irq, NULL, demo_irq_handler,
                    IRQF_ONESHOT | IRQF_TRIGGER_FALLING, "demo", NULL);
    if (ret) {
        printk("Failed to acquire irq %d\n", irq);
        return -EINVAL;
    }

    return 0;
}

/* remove platform driver */
static int demo_remove(struct platform_device *pdev)
{
    /* Release Interrupt */
    free_irq(irq, NULL);

    return 0;
}

static const struct of_device_id demo_of_match[] = {
    { .compatible = "demo,demo_intr", },
    { },
};
MODULE_DEVICE_TABLE(of, demo_of_match);

/* platform driver information */
static struct platform_driver demo_driver = {
    .probe  = demo_probe,
    .remove = demo_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = DEV_NAME, /* Same as device name */
        .of_match_table = demo_of_match,
    }, 
};

module_platform_driver(demo_driver);
