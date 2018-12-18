/*
 * Can demo driver
 *
 * (C) 2018.12.18  BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/can/core.h>
#include <linux/can/dev.h>

#define TX_ECHO_SKB_MAX  1

static struct net_device *can_net;

/* 'ifconfig canx up' will invoke this function firstly. */
static int demo_open(struct net_device *net)
{
    /* Open can device and do some specify operation. */
    return 0;
}

/* 'ifconfig canx down' will invoke this function firstly. */
static int demo_stop(struct net_device *net)
{
    /* Close can device and do some release operation. */
    return 0;
}

static netdev_tx_t demo_hard_start_xmit(struct sk_buff *skb, 
                                              struct net_device *dev)
{
    /* To do Can transfer data */
    return NETDEV_TX_OK;
}

static const struct net_device_ops demo_netdev_ops = {
    .ndo_open   = demo_open,
    .ndo_stop   = demo_stop,
    .ndo_start_xmit = demo_hard_start_xmit,
};

static int __init demo_can_device_init(void)
{
    int ret;

    /* Allocate can/net device */
    can_net = alloc_candev(sizeof(*can_net), TX_ECHO_SKB_MAX);
    if (!can_net) {
        ret = -ENOMEM;
        goto error_alloc;
     }

    can_net->netdev_ops = &demo_netdev_ops; /* necessary */

    /* register can device */
    ret = register_candev(can_net);
    if (ret) {
        ret = -EINVAL;
        goto error_register_dev;
    }
    
    return 0;

error_register_dev:
    free_candev(can_net);
error_alloc:
    return ret;
}

static void __exit demo_can_device_exit(void)
{
    /* Unregister can device */
    unregister_candev(can_dev);

    /* release can device */
    free_candev(can_net);
}

module_init(demo_can_device_init);
module_exit(demo_can_device_exit);
MODULE_LICENSE("GPL v2");
