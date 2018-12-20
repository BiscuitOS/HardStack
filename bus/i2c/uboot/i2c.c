/*
 * I2C read/write on Uboot
 *
 * (C) 2018.11.17 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <common.h>
#include <i2c.h>
#include <errno.h>

/* I2C write */
static int i2c_demo_write(uint i2c_addr, ulong offset, uchar value)
{
    i2c_write(i2c_addr, offset, 1, &value, 1);

    return 0;
}

/* I2C read */
static int i2c_demo_read(uint i2c_addr, ulong offset, uchar &value)
{
    i2c_read(i2c_addr, offset, 1, &value, 1);

    return 0;
}

