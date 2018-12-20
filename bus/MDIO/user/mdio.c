/*
 * MDIO Userland Procedure-Interface
 *
 * (C) 2018.12.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include <linux/err.h>
#include <linux/io.h>
#include <asm/uaccess.h>
#include <linux/bitops.h>
#include <linux/phy.h>

#define DEV_NAME "mdio_demo"

/* mdio_demo from real Ethernet card, and export it here! */
extern struct mii_bus *mdio_demo;

/* Parse input string
 *  Value[0]: DeviceAddress. Value[1]: Register. Value[2]: date in Byte.
 *  
 *  Read/Write operation 
 *  CMD: <r/w>,<DevAddr>,<RegAddr>,[Value],
 *       r: Read special PHY/SERDES register
 *       w: Write data to special PHY/SERDES register.
 *       DevAddr: Device address 
 *       RegAddr: Register address
 *       Value:   value what to write.
 */
static int parse_input_string(const char *string, int *value, int *flag)
{
    int nr;
    char *tmp;
    char *buffer, *leg;
    int i = 0;

    buffer = (char *)kmalloc(strlen(string) + 1, GFP_KERNEL);
    leg = buffer;
    memset(buffer, 0, strlen(string) + 1);
    /* Copy original data */
    strcpy(buffer, string);

    while ((tmp = strstr(buffer, ","))) {
        int data;
        char dd[20];

        nr = tmp - buffer;
        tmp++;
        strncpy(dd, buffer, nr);
        dd[nr] = '\0';
        if (strncmp(dd, "r", 1) == 0) {
            *flag = 1;
        } else if (strncmp(dd, "w", 1) == 0) {
            *flag = 2;
        } else {
            sscanf(dd, "%d", &data);
            value[i++] = data;
        }
        buffer = tmp;
    }
    kfree(leg);
    return 0;
}

/* Dump all PHY's all register */
static ssize_t mdio_demo_show(struct device *dev,
                    struct device_attribute *attr, char *buf)
{
    ssize_t size = 0;
    int phy, reg;

    for (phy = 0; phy < 32; phy++) {
        for (reg = 0; reg < 32; reg++) {
            unsigned short val;

            if (((reg % 16) == 0) && (reg != 0))
                printk("\n");
            val = mdio_demo->read(mdio_demo, phy, reg);
            printk("%#04x ", val);
        }
    }
    printk("\n");

    return size;
}

/* Read/Write PHY register */
static ssize_t mdio_demo_store(struct device *dev,
            struct device_attribute *attr, const char *buf, size_t size)
{
    int flag = 0; /* 1: read 2: write */
    int value[10];

    parse_input_string(buf, value, &flag);
    /* Read data from Port-Register */
    if (flag == 1) {
        unsigned short reg;

        reg = mdio_demo->read(mdio_demo, value[0], value[1]);
        /* Output message into dmesg */
        printk("\r\nRead: Port - Dev[%#x] Reg[%#x] Value[%#x]\n", 
                                              value[0], value[1], reg);
    } else if (flag == 2) { /* Write data to Port-Register */

        mdio_demo->write(mdio_demo, value[0], value[1], value[2]);
        /* Output message into dmesag */
        printk("\r\nWrite: Port - Dev[%#x] Reg[%#x] value[%#x]\n", 
                                          value[0], value[1], value[2]);
    } else {
        printk(KERN_ERR "Unknown operation from Port register\n");
    }
 
    return size;
}

static struct device_attribute mdio_demo_attr = 
       __ATTR_RW(mdio_demo);

/* probe platform driver */
static int mdio_demo_probe(struct platform_device *pdev)
{
    int err;

    err = device_create_file(&pdev->dev, &mdio_demo_attr);
    if (err) {
        printk("Unable to create device file for reg***.\n");
        return -EINVAL;
    }

    return 0;
}

/* remove platform driver */
static int mdio_demo_remove(struct platform_device *pdev)
{
    device_remove_file(&pdev->dev, &mdio_demo_attr);

    return 0;
}

/* platform device information */
static struct platform_device mdio_demo_device = {
    .name = DEV_NAME,  /* Same as driver name */
    .id   = -1,
};

/* platform driver information */
static struct platform_driver mdio_demo_driver = {
    .probe  = mdio_demo_probe,
    .remove = mdio_demo_remove,
    .driver = {
        .name = DEV_NAME, /* Same as device name */
    }, 
};

/* init entence */
static __init int mdio_demo_init(void)
{
    int ret;


    /* register device */
    ret = platform_device_register(&mdio_demo_device);
    if (ret)
        return ret;    
    
    /* register driver */
    return platform_driver_register(&mdio_demo_driver);
}

/* Exit entence */
static __exit void mdio_demo_exit(void)
{
    platform_driver_unregister(&mdio_demo_driver);
    platform_device_unregister(&mdio_demo_device);
}

module_init(mdio_demo_init);
module_exit(mdio_demo_exit);

MODULE_LICENSE("GPL v2");
