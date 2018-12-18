/*
 * Perfect Can driver
 *
 * (C) 2018.12.18 BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define CAN_FRAME_MAX_DATA_LEN     8
#define DEMO_TRANSFER_BUF_LEN      (6 + CAN_FRAME_MAX_DATA_LEN)
#define TX_ECHO_SKB_MAX            1
#define DEVICE_NAME                "demo_can"

/* Can bus frequency */
#define CAN_BUS_FREQUENCY    (1*1024*1024)

static struct net_device *can_net;

/* can device private data */
struct demo_can_private_data 
{
    struct can_priv can;    /* can device private data, must be first member */
    struct net_device *net; /* net device */

    /* work queue */
    struct workqueue_struct *wq;
    struct work_struct tx_work;
    struct work_struct restart_work;

    /* trans */
    struct sk_buff *tx_skb;
    int tx_len;

    /* flags */
    int force_quit;
    int after_suspend;
    int restart_tx;
};

/* Can hardware-depends bit-timing */
static struct can_bittiming_const demo_bittiming_const = {
    .name        = DEVICE_NAME,
    .tseg1_min   = 3,
    .tseg1_max   = 16,
    .tseg2_min   = 2,
    .tseg2_max   = 8,
    .sjw_max     = 4,
    .brp_min     = 1,
    .brp_max     = 64,
    .brp_inc     = 1,
};

/* parse can frame and trans */
static void demo_hw_tx(struct demo_can_private_data *priv, 
       struct can_frame *frame, int tx_buf_idx)
{
    u32 sid, eid, exide, rtr;

    /* parse can frame */
    exide = (frame->can_id & CAN_EFF_FLAG) ? 1 : 0; /* Extended ID Enable */
    if (exide)
        sid = (frame->can_id & CAN_EFF_MASK) >> 18;
    else
        sid = frame->can_id & CAN_EFF_MASK; /* Standard ID */
    eid = frame->can_id & CAN_EFF_MASK;     /* Extended ID */
    rtr = (frame->can_id & CAN_RTR_FLAG) ? 1 : 0; /* Remote transmission */

    /* prase can frame done. */
}

/* tx work queue handler */
static void demo_tx_work_handler(struct work_struct *ws)
{
    struct demo_can_private_data *priv = container_of(ws, 
                 struct demo_can_private_data, tx_work);
    struct net_device *net = priv->net;
    struct can_frame *frame;

    if (priv->tx_skb) {
        if (priv->can.state == CAN_STATE_BUS_OFF) {
        } else {
            frame = (struct can_frame *)priv->tx_skb->data;

            if (frame->can_dlc > CAN_FRAME_MAX_DATA_LEN)
                frame->can_dlc = CAN_FRAME_MAX_DATA_LEN;
            /* process can frame */
            demo_hw_tx(priv, frame, 0);
            priv->tx_len = 1 + frame->can_dlc;
            can_put_echo_skb(priv->tx_skb, net, 0);
            priv->tx_skb = NULL;
        }
    }
}

/* restart work queue */
static void demo_restart_work_handler(struct work_struct *ws)
{
    struct demo_can_private_data *priv = 
           container_of(ws, struct demo_can_private_data, restart_work);
    struct net_device *net = priv->net;

    if (priv->after_suspend) {
        priv->after_suspend = 0;
        priv->force_quit    = 0;
    }

    if (priv->restart_tx) {
        priv->restart_tx = 0;
        netif_wake_queue(net);
    }
}

/* 'ifconfig canx up' will invoke this function firstly. */
static int demo_open(struct net_device *net)
{
    /* Open can device and do some specify operation. */
    struct demo_can_private_data *priv = netdev_priv(net);
    int ret;

    /* open can device*/
    ret = open_candev(net);
    if (ret)
        return ret;

    /* clear flags */
    priv->force_quit = 0;
    priv->tx_skb = NULL;
    priv->tx_len = 0;
    
    /* request work queue to process can frame */
    priv->wq = create_freezable_workqueue("demo_can_wq");
    INIT_WORK(&priv->tx_work, demo_tx_work_handler);
    INIT_WORK(&priv->restart_work, demo_restart_work_handler);

    /* hardware initialization */

    /* refresh can state */
    priv->can.state = CAN_STATE_ERROR_ACTIVE;
    /* wakeup net queue */
    netif_wake_queue(net);

    return ret;
}

/* 'ifconfig canx down' will invoke this function firstly. */
static int demo_stop(struct net_device *net)
{
    /* Close can device and do some release operation. */
    return 0;
}

/* To do can transfer data */
static netdev_tx_t demo_hard_start_xmit(struct sk_buff *skb,
                   struct net_device *net)
{
    struct demo_can_private_data *priv = netdev_priv(net);

    if (priv->tx_skb || priv->tx_len)
        return NETDEV_TX_BUSY;

    if (can_dropped_invalid_skb(net, skb))
        return NETDEV_TX_OK;

    netif_stop_queue(net);
    priv->tx_skb = skb;
    queue_work(priv->wq, &priv->tx_work);
    return NETDEV_TX_OK;
}

static const struct net_device_ops demo_netdev_ops = {
    .ndo_open   = demo_open,
    .ndo_stop   = demo_stop,
    .ndo_start_xmit = demo_hard_start_xmit,
};

/*
 * Set can mode.
 *  Can device support CAN_MODE_START, CAN_MODE_STOP and CAN_MODE_SLEEP.
 * @net: net device
 * @mode: can mode that be setup. 
 */
static int demo_do_set_mode(struct net_device *net, enum can_mode mode)
{
    switch (mode) {
    case CAN_MODE_START:
        break;
    case CAN_MODE_STOP:
        break;
    case CAN_MODE_SLEEP:
        break;
    default:
        return -EOPNOTSUPP;
    }
    return 0;
}

static __init int demo_can_device_init(void)
{
    struct net_device *net;
    struct demo_can_private_data *priv;
    int ret;

    /* Allocate can/net device */
    net = alloc_candev(sizeof(struct demo_can_private_data), TX_ECHO_SKB_MAX);
    if (!net) {
        ret = -ENOMEM;
        goto error_alloc;
    }

    net->netdev_ops = &demo_netdev_ops; /* necessary */
    net->flags |= IFF_ECHO;

    /* get private data for can device */
    priv = netdev_priv(net);
    can_net = net;
    /* Setup can bit-timing parameters */

    /* Setup can hardware-dependent bit-timing */
    priv->can.bittiming_const = &demo_bittiming_const;
    /* Setup interface for can mode */
    priv->can.do_set_mode = demo_do_set_mode;
    /* Setup can bus frequence. */
    priv->can.clock.freq = CAN_BUS_FREQUENCY;
    /* Setup can device control mode */
    priv->can.ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
          CAN_CTRLMODE_LOOPBACK |  CAN_CTRLMODE_LISTENONLY;
    /* callback */
    priv->net = net;

    /* register can device */
    ret = register_candev(net);
    if (ret) {
        ret = -EINVAL;
        goto error_register_dev;
    }
    
    return 0;

error_register_dev:
    free_candev(net);
error_alloc:
    return ret;
}

static void __exit demo_can_device_exit(void)
{
    /* Unregister can device */
    unregister_candev(can_net);

    /* release can device */
    free_candev(can_net);
}

module_init(demo_can_device_init);
module_exit(demo_can_device_exit);
MODULE_LICENSE("GPL");
