/*
 * AT24C08 Multip Slave Address
 *
 * (C) 2019.10.28 <buddy.zhang@aliyun.com>
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
#define I2C_ADDR0	0x50
#define I2C_ADDR1	0x51
#define I2C_ADDR2	0x52
#define I2C_ADDR3	0x53
#define I2C_M_WR	0

/* Random Read
 *
 * SDA LINE
 *
 *
 *  S                                     S
 *  T                                     T               R               S
 *  A                                     A               E               T
 *  R                                     R               A               O
 *  T                                     T               D               P
 * +-+-+ +-+ +-+-+-+ + +-+-+-+-+-+-+-+-+-+-+-+ +-+ +-+-+-+-+-+-+-+-+-+-+-+-+
 * | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 * | | | | | |     | | |*              | | | | | | |     | | |  ...  | | | |
 * | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 * +-+ +-+ +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ +-+ +-+-+-+-+ +-+-+-+-+-+-+ +-+
 *    M           L R A M             L A   M           L   A  Data n   N
 *    S           S / C S             S C   S           S   C  (8bits)  O
 *    B           B W K B             B K   B           B   K
 * |                                   |                                A
 * | <-------------------------------> |                                C
 *              DUMMY WRITE                                             K
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
static int Random_read(int fd, unsigned char addr, 
				unsigned char offset, unsigned char *buf)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs[2];
	int rc;

	msgs[0].addr	= addr & 0xfe;
	msgs[0].flags	= I2C_M_WR;
	msgs[0].len	= 1;
	msgs[0].buf	= &offset;

	msgs[1].addr	= addr & 0xfe;
	msgs[1].flags	= I2C_M_RD;
	msgs[1].len	= 1;
	msgs[1].buf	= buf;

	msgset.msgs	= msgs;
	msgset.nmsgs	= 2;

	rc = ioctl(fd, I2C_RDWR, &msgset);
	if (rc != 2)
		printf("Loss packet %d on Random Read\n", rc);
	return rc;
}

/* Byte Write
 *
 *
 *  S               W
 *  T               R                                       S
 *  A               I                                       T
 *  R  DEVICE       T                                       O
 *  T ADDRESS       E    WORD ADDRESS          DATA         P
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
static int Byte_write(int fd, unsigned char addr, unsigned char offset, 
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
	char msg = 0x89;
	char buf[8];
	int fd, ret;
	int idx;

	/* Open I2C Bus */
	fd = open(I2C_BUS, O_RDWR);
	if (fd < 0) {
		printf("Unable to open %s\n", I2C_BUS);
		return -1;
	}

	/********** Write Operation **********/

	/* Byte write */
	memset(buf, 0, 8);
	buf[0] = 0x60;
	buf[1] = 0x61;
	buf[2] = 0x63;
	buf[3] = 0x66;

	/* Write to 1st 2K */
	ret = Byte_write(fd, I2C_ADDR0, 0x00, buf[0]);
	if (ret < 0)
		goto err;
	usleep(10000);
	/* Write to 2nd 2K */
	ret = Byte_write(fd, I2C_ADDR1, 0x00, buf[1]);
	if (ret < 0)
		goto err;
	usleep(10000);
	/* Write to 3rd 2K */
	ret = Byte_write(fd, I2C_ADDR2, 0x00, buf[2]);
	if (ret < 0)
		goto err;
	usleep(10000);
	/* Write to 4th 2K */
	ret = Byte_write(fd, I2C_ADDR3, 0x00, buf[3]);
	if (ret < 0)
		goto err;
	usleep(10000);

	/* Random Read 1st 2K */
	memset(buf, 0, 8);
	ret = Random_read(fd, I2C_ADDR0, 0x00, buf);
	if (ret < 0)
		goto err;
	printf("1st 2K-space 0x0 DATA: %#x\n", buf[0]);
	/* Random Read 2nd 2K */
	memset(buf, 0, 8);
	ret = Random_read(fd, I2C_ADDR1, 0x00, buf);
	if (ret < 0)
		goto err;
	printf("2nd 2K-space 0x0 DATA: %#x\n", buf[0]);
	/* Random Read 3rd 2K */
	memset(buf, 0, 8);
	ret = Random_read(fd, I2C_ADDR2, 0x00, buf);
	if (ret < 0)
		goto err;
	printf("3rd 2K-space 0x0 DATA: %#x\n", buf[0]);
	/* Random Read 4th 2K */
	memset(buf, 0, 8);
	ret = Random_read(fd, I2C_ADDR3, 0x00, buf);
	if (ret < 0)
		goto err;
	printf("4st 2K-space 0x0 DATA: %#x\n", buf[0]);

	/* Close I2C Bus */
	close(fd);

	return 0;

err:
	printf("ERROR\n");
	close(fd);
	return ret;
}
