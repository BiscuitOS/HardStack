/*
 * I2C demo on userspace
 *
 * (C) 2018.12.14 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define I2C_SLAVE_ADDR      0x50
#define I2C_BUS             "/dev/i2c-0"
#define I2C_M_WR            0x0

/*
 * I2C read
 * @fd: file handler
 * @addr: i2c slave 7-bit address
 * @offset: read position.
 * @buf: buffer for reading data.
 * @len: length for reading.
 *
 * @return: the number of i2c_msg has read. 
 *          succeed is 2.
 */
int I2CBus_packetRead(int fd, unsigned char addr, unsigned char offset,
                             unsigned char *buf, unsigned char len)
{
    unsigned char tmpaddr[2];
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset;
    int rc;

    tmpaddr[0]     = offset;
    msgs[0].addr   = addr & 0xfe;
    msgs[0].flags  = I2C_M_WR;
    msgs[0].len    = 1;
    msgs[0].buf    = tmpaddr;

    msgs[1].addr   = addr & 0xfe;
    msgs[1].flags  = I2C_M_RD;  ;
    msgs[1].len    = len;
    msgs[1].buf    = buf;

    msgset.msgs    = msgs;
    msgset.nmsgs   = 2;

    rc = ioctl(fd, I2C_RDWR, &msgset);
    return rc;
}

/* 
 * I2C write
 * @fd: file handler.
 * @addr: i2c slave 7-bit address
 * @offset: write position
 * @buf: buffer for writuing data.
 * @len: the length for writing
 *
 * @return: the number of i2c_msg has write.
 *          succeed is 1.
 */
int I2CBus_packetWrite(int fd, unsigned char addr, unsigned char offset,
                              unsigned char *buf, unsigned char len)
{
    unsigned char tmpaddr[2];
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset;
    int rc;

    tmpaddr[0]     = offset;
    tmpaddr[1]     = buf[0];
    msgs[0].addr   = addr & 0xfe;
    msgs[0].flags  = I2C_M_WR;
    msgs[0].len    = 2;
    msgs[0].buf    = tmpaddr;

    msgset.msgs    = msgs;
    msgset.nmsgs   = 1;

    rc = ioctl(fd, I2C_RDWR, &msgset);
    return rc;
}

int main()
{
    unsigned char value;
    int fd;

    fd = open(I2C_BUS, O_RDWR);
    if (fd < 0) {
        printf("Unable to open I2C Bus.\n");
        return -1;
    }

    /* Read Data from I2C */
    I2CBus_packetRead(fd, I2C_SLAVE_ADDR, 0x20, &value, 1);

    /* Write Dato into I2C */
    I2CBus_packetWrite(fd, I2C_SLAVE_ADDR, 0x20, &value, 1);

    return 0;
}
