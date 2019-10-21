/*
 * AT24C08
 *
 * (C) 2019.10.13 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define I2C_BUS		"/dev/i2c-1"
#define I2C_ADDR	0x50
#define I2C_M_WR	0

static int I2CBus_packageRead(int fd, unsigned char addr, unsigned char offset,
				unsigned char *buf, unsigned char len)
{
	unsigned char tmpaddr[2];
	struct i2c_msg msgs[2];
	struct i2c_rdwr_ioctl_data msgset;
	int rc;

	tmpaddr[0]	= offset;
	msgs[0].addr	= addr & 0xfe;
	msgs[0].flags	= I2C_M_WR;
	msgs[0].len	= 1;
	msgs[0].buf	= tmpaddr;

	msgs[1].addr	= addr & 0xfe;
	msgs[1].flags	= I2C_M_RD;
	msgs[1].len	= len;
	msgs[1].buf	= buf;

	msgset.msgs	= msgs;
	msgset.nmsgs	= 2;

	rc = ioctl(fd, I2C_RDWR, &msgset);
	return rc;
}

static int I2CBus_packageWrite(int fd, unsigned char addr, 
		unsigned char offset, unsigned char *buf, unsigned char len)
{
	unsigned char tmpaddr[2];
	struct i2c_msg msgs[2];
	struct i2c_rdwr_ioctl_data msgset;
	int rc;

	tmpaddr[0]	= offset;
	tmpaddr[1]	= buf[0];
	msgs[0].addr	= addr & 0xfe;
	msgs[0].flags	= I2C_M_WR;
	msgs[0].len	= 2;
	msgs[0].buf	= tmpaddr;

	msgset.msgs	= msgs;
	msgset.nmsgs	= 1;

	rc = ioctl(fd, I2C_RDWR, &msgset);
	return rc;
}

int main()
{
	char msg = 0x89;
	char buf[8];
	int fd, rvl;

	/* Open I2C Bus */
	fd = open(I2C_BUS, O_RDWR);
	if (fd < 0) {
		printf("Unable to open %s\n", I2C_BUS);
		return -1;
	}
	
	memset(buf, 0, 8);
	/* Read Operations */
	rvl = I2CBus_packageRead(fd, I2C_ADDR, 0x0, buf, 1);
	if (rvl < 0) {
		printf("I2C Read error.\n");
		close(fd);
		return -1;
	}
	printf("DATA: %#x\n", buf[0]);

	/* Close I2C Bus */
	close(fd);

	return 0;
}
