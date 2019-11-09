/*
 * PCF8574AT 8-bit I/O expander Application
 *
 * (C) 2019.10.29 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define I2C_BUS		"/dev/i2c-1"
#define I2C_ADDR	0x38
#define I2C_M_WR	0

#define GREEN_ON	0x01
#define RED_ON		0x02
#define YELLO_ON	0x04
#define LIGHT_OFF	0x00
#define TRAFFIC_ADDR	0xFF

/* Read
 *
 * SDA LINE
 *
 *
 *  S
 *  T               R                           S
 *  A               E                           T
 *  R               A                           O
 *  T               D                           P
 * +-+-+ +-+ +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | | | | | | | | | | | | | | | | | | | | | | | |
 * | | | | | |     | | |  ...  | | |  ...  | | | |
 * | | | | | | | | | | | | | | | | | | | | | | | |
 * +-+ +-+ +-+-+-+-+ +-+-+-+-+-+-+-+-+-+-+-+-+ +-+
 *    M           L   A  Data n   A  Data n   N
 *    S           S   C  (8bits)  C  (8bits)  O
 *    B           B   K           K
 *                                            A
 *                                            C
 *                                            K
 *
 */
static int pcf8574a_read(int fd, unsigned char addr, 
				unsigned char offset, unsigned char *buf)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs;
	int rc;

	msgs.addr	= addr & 0xfe;
	msgs.flags	= I2C_M_RD;
	msgs.len	= 2;
	msgs.buf	= buf;

	msgset.msgs	= &msgs;
	msgset.nmsgs	= 1;

	rc = ioctl(fd, I2C_RDWR, &msgset);
	if (rc != 1)
		printf("Loss packet %d on Random Read\n", rc);
	return rc;
}

/* Write
 *
 *
 *  S               W
 *  T               R                                       S
 *  A               I                                       T
 *  R  DEVICE       T                                       O
 *  T ADDRESS       E        DATA              DATA         P
 * +-+-+ +-+ +-+-+-+ + +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+ +-+
 * | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 * | | | | | |     | | |*              | |               | | |
 * | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 * +-+ +-+ +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    M           L R A   M           L A
 *    S           S / C   S           S C
 *    B           B W K   B           B K
 *
 */
static int pcf8574a_write(int fd, unsigned char addr, unsigned char offset, 
					unsigned char data)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs;
	unsigned char tmp[2];
	int rc;

	tmp[0]		= offset;
	tmp[1]		= data;
	msgs.addr	= addr & 0xfe;
	msgs.flags	= I2C_M_WR;
	msgs.len	= 2;
	msgs.buf	= tmp;

	msgset.msgs	= &msgs;
	msgset.nmsgs	= 1;

	rc = ioctl(fd, I2C_RDWR, &msgset);
	if (rc != 1)
		printf("Loss packet %d on Byte write\n", rc);
	return rc;
}

int main()
{
	char buf[2];
	int fd, ret;
	int idx;

	/* Open I2C Bus */
	fd = open(I2C_BUS, O_RDWR);
	if (fd < 0) {
		printf("Unable to open %s\n", I2C_BUS);
		return -1;
	}

	while (1) {
		/* Traffic light */
	
		/* Green light hold 2s */
		pcf8574a_write(fd, I2C_ADDR, TRAFFIC_ADDR, GREEN_ON);
		sleep(2);
		pcf8574a_write(fd, I2C_ADDR, TRAFFIC_ADDR, LIGHT_OFF);

		/* Yello light flash 2s */
		pcf8574a_write(fd, I2C_ADDR, TRAFFIC_ADDR, YELLO_ON);
		sleep(1);
		pcf8574a_write(fd, I2C_ADDR, TRAFFIC_ADDR, LIGHT_OFF);
		sleep(1);
		pcf8574a_write(fd, I2C_ADDR, TRAFFIC_ADDR, YELLO_ON);
		sleep(1);
		pcf8574a_write(fd, I2C_ADDR, TRAFFIC_ADDR, LIGHT_OFF);

		/* Red light hold 2s */
		pcf8574a_write(fd, I2C_ADDR, TRAFFIC_ADDR, RED_ON);
		sleep(2);
		pcf8574a_write(fd, I2C_ADDR, TRAFFIC_ADDR, LIGHT_OFF);
	}

	/* Close I2C Bus */
	close(fd);

	return 0;
}
