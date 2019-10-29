/*
 * PCF8574 Userland
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

/* GPIO Mapping */
#define GPIO0		0x01
#define GPIO1		0x02
#define GPIO2		0x04
#define GPIO3		0x08
#define GPIO4		0x10
#define GPIO5		0x20
#define GPIO6		0x40
#define GPIO7		0x80
#define GPIO_MASK	0xFF

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
 * +-+ +-+ +-+-+-+-+ +-+-+-+-+-+-+ +-+-+-+-+-+ +-+
 *    M           L   A  Data n   A  Data n   N 
 *    S           S   C  (8bits)  C  (8bits)  O
 *    B           B   K           K           
 *                                            A
 *                                            C
 *                                            K
 *    
 *
 * (* = DON't CARE bit for 1K)
 *    
 *
 * A random read requires a "dummy" byte write sequence to load in the
 * data word address. Once the device address word and data word address
 * are clocked in and acknowledged by the EEPROM, the microcontroller
 * must generate another start condition. The microcontroller now initiates
 * a current address read by sending a device address with the read/write
 * select bit high. The EEPROM acknowledges the device address and serially
 * clocks out the data word. The microcontroller does not respond with a
 * zero but does generate a following stop condition.
 */
static int pcf8574_read(int fd, unsigned char addr, 
				unsigned char offset, unsigned char *buf)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs;
	int rc;

	msgs.addr	= addr & 0xfe;
	msgs.flags	= I2C_M_RD;
	msgs.len	= 0;
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
 *  T ADDRESS       E       P7 - P0          P7 - P0 DATA   P
 * +-+-+ +-+ +-+-+-+ + +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+ +-+
 * | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 * | | | | | |     | | |*              | |               | | |
 * | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 * +-+ +-+ +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    M           L R A   M           L A
 *    S           S / C   S           S C
 *    B           B W K   B           B K
 *
 *
 * A write operation requires an 8-bit data word address following the
 * device address and acknowledgment. Upon receipt of this address,
 * the EEPROM will again respond with a zero and then clock in the first
 * 8-bit data word. Following receipt of the 8-bit data word, the EEPROM
 * will output a zero and the addressing device, such as a microcontroller,
 * must terminate the write sequence with a stop condition. At this time
 * the EEPROM enters an internally timed write cycle, t(WR), to the 
 * nonvolatile memory. All inputs are disabled during this write cycle
 * and the EEPROM will not respond until the write is complete.
 *
 */
static int pcf8574_write(int fd, unsigned char addr, unsigned char offset, 
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

	/* Write operation */
	pcf8574_write(fd, I2C_ADDR, GPIO5, 1);

	memset(buf, 0, 2);
	/* Read operation */
	pcf8574_read(fd, I2C_ADDR, GPIO5, buf);
	printf("Buf[0] %hhx\n", buf[0]);
	printf("Buf[1] %hhx\n", buf[1]);

	/* Close I2C Bus */
	close(fd);

	return 0;
}
