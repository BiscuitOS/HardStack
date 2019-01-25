/*
 * delay or sleep
 *
 * (C) 2019.01.25 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/delay.h> /* mdelay(), msleep(), ssleep() */

static __init int demo_delay_init(void)
{
    printk("Dealy Procedure Entence...\n");

    /* millisencond(ms) 1s = 1000 ms */
    mdelay(1000);
    printk("Delay 1000ms done.\n");

    /* msleep */
    msleep(1000);
    printk("Sleep 1000ms done.\n");

    /* ssleep */
    ssleep(1);
    printk("Sleep 1s done.\n");
    
    return 0;
}
arch_initcall(demo_delay_init);
