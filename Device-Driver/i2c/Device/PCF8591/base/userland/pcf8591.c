/*
 * PCF8591 A/D D/A converter Application
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
#define I2C_ADDR	0x48
#define I2C_M_WR	0

#define PCF8591_MODE0	0x00
#define PCF8591_MODE1	0x10
#define PCF8591_MODE2	0x20
#define PCF8591_MODE3	0x30
#define MODE_MASK	0x30

#define PCF8591_CHANEL0	0x00
#define PCF8591_CHANEL1	0x01
#define PCF8591_CHANEL2	0x02
#define PCF8591_CHANEL3	0x03

#define PCF8591_INCREMENT	0x04
#define PCF8591_OUTPUT_ENABLE	0x40

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
static int pcf8591_read(int fd, unsigned char addr, 
				unsigned len, unsigned char *buf)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs;
	int rc;

	msgs.addr	= addr & 0xfe;
	msgs.flags	= I2C_M_RD;
	msgs.len	= len;
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
static int pcf8591_write(int fd, unsigned char addr, unsigned char contrl, 
			unsigned char len, unsigned char *buf)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs;
	unsigned char *tmp;
	int rc;

	tmp = malloc(len + 1);
	tmp[0] = contrl;
	memcpy(&tmp[1], buf, len);

	msgs.addr	= addr & 0xfe;
	msgs.flags	= I2C_M_WR;
	msgs.len	= len + 1;
	msgs.buf	= tmp;

	msgset.msgs	= &msgs;
	msgset.nmsgs	= 1;

	rc = ioctl(fd, I2C_RDWR, &msgset);
	if (rc != 1)
		printf("Loss packet %d on Byte write\n", rc);
	free(tmp);
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

	buf[0] = 0x10;
	pcf8591_write(fd, I2C_ADDR, 
			PCF8591_MODE0 | PCF8591_CHANEL0,
					1, &buf[0]);

	memset(buf, 0, 2);
	pcf8591_read(fd, I2C_ADDR, 2, buf);
	printf("Buf[0] %#hhx\n", buf[0]);
	printf("Buf[1] %#hhx\n", buf[1]);
	sleep(1);
	/* Close I2C Bus */
	close(fd);

	return 0;
}
