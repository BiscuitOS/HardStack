/*
 * I2C read/write on Kernel
 *
 * (C) 2018.11.17 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of.h>

#define DEV_NAME         "i2c_demo"
#define SLAVE_I2C_ADDR   0x50

/*
 * Slave information on DTS
 *   I2C Bus:    I2C 0
 *   Slave addr: 0x50
 *
 * &i2c0 {
 * 	status = "okay";
 * 		i2cdemo@50 {
 * 			compatible = "i2cdemo,eeprom";
 *      		reg = <0x50>;           
 * 		};
 * };
 */

/*
 * Write data into Slave device on I2C Bus.
 *
 * client: Slave client.
 * reg: offset on slave device.
 * data: need to write.
 * count: number for write.
 */
static int i2c_demo_write(struct i2c_client *client, unsigned char reg,
                                 unsigned char *data, unsigned long count)
{
    int ret;

    struct i2c_msg msgs[] = {
        {
            .addr  = client->addr,
            .flags = client->flags & I2C_M_TEN,
            .len   = 1,
            .buf   = &reg, /* Offset address on slave device */
        },
        {
            .addr  = client->addr,
            .flags = client->flags,
            .len   = count,
            .buf   = data, /* value need to write */
        }
    };

    ret = i2c_transfer(client->adapter, msgs, 2);
    if (2 != ret) {
        printk(KERN_ERR "%s fail\n", __func__);
        return -1;
    }

    return 0;
}

/*
 * Read data from slave device on I2C Bus.
 *
 * client: Slave client
 * reg: offset on slave device.
 * data: store ready data.
 * count: number for read.
 */
static int i2c_demo_read(struct i2c_client *client, unsigned char reg,
                           unsigned char *data, unsigned long count)
{
    int ret;

    struct i2c_msg msgs[] = {
        {
            .addr  = client->addr,
            .flags = client->flags & I2C_M_TEN,
            .len   = 1,
            .buf   = &reg, /* Offset address on slave device */
        },
        {
            .addr  = client->addr,
            .flags = client->flags | I2C_M_RD,
            .len   = count,
            .buf   = data, /* value need to write */
        }
    };

    ret = i2c_transfer(client->adapter, msgs, 2);
    if (2 != ret) {
        printk(KERN_ERR "%s fail\n", __func__);
        return -1;
    }

    return 0;
}

/* probe entence */
static int i2c_demo_probe(struct i2c_client *client,
                            const struct i2c_device_id *id)
{
    unsigned char addr = 0x20;
    unsigned char buf;

    /* Read data from I2C Bus */
    i2c_demo_read(client, addr, &buf, 1);
    printk(KERN_INFO "Origin-Data: %#x\n", (unsigned int)buf);

    buf = 0x68;
    /* Write data into I2C Bus */
    i2c_demo_write(client, addr, &buf, 1);

    /* Read data from I2C Bus */
    i2c_demo_read(client, addr, &buf, 1);
    printk(KERN_INFO "Modify-Data: %#x\n", (unsigned int)buf);
    
    return 0;
}

/* remove entence */
static int i2c_demo_remove(struct i2c_client *client)
{
    return 0;
}

static struct of_device_id i2c_demo_match_table[] = {
    { .compatible = "i2cdemo,eeprom", },
    { },
};

static const struct i2c_device_id i2c_demo_id[] = {
    { DEV_NAME, SLAVE_I2C_ADDR},
    {},
};

static struct i2c_driver i2c_demo_driver = {
    .driver = {
        .name  = DEV_NAME,
        .owner = THIS_MODULE,
        .of_match_table = i2c_demo_match_table,
    },
    .probe    = i2c_demo_probe,
    .remove   = i2c_demo_remove,
    .id_table = i2c_demo_id,
};

module_i2c_driver(i2c_demo_driver);

/* Module information */
MODULE_DESCRIPTION("i2c demo");
MODULE_LICENSE("GPL v2");
