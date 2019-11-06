/*
 * LM75A Userland
 *
 * (C) 2019.10.30 <buddy.zhang@aliyun.com>
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

/* Register Mapping */
#define LM75A_TEMPER_REG	0x00
#define LM75A_CONFIG_REG	0x01
#define LM75A_THYST_REG		0x02
#define LM75A_TOS_REG		0x03

#define TEMP_MSB_MASK		0xFF
#define TEMP_LSB_MASK		0x07

/* Configuration Register Read
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
static int lm75a_read(int fd, unsigned char addr, 
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

/* Temp or Tos or Thyst Register read
 *
 * SDA LINE
 *
 *
 *          R                           S
 *          E             A             T
 * DEVICE   A             C             O
 * ADDRESS  D             K             P
 * - - - - +-+ +-+-+-+-+-+ +-+-+-+-+-+-+-+
 *       | | | | | | | | | | | | | | | | |
 *         | | |  ...  | | |  ...  | | | |
 *       | | | | | | | | | | | | | | | | |
 * - - - - + +-+-+-+-+-+-+-+-+-+-+-+-+ +-+
 *          R A   MSB         LSB     N
 *          / C                       O
 *          W K
 *                                    A
 *                                    C
 *                                    K
 *
 *
 * Sequential reads are initated by either a current address read or a
 * random address read. After the microcontroller receives a data word,
 * it responds with an acknowledge. As long as the EEPROM receives an 
 * acknowledge, it will continue to increment the data word address and
 * serially clock out sequential data words. When the memory address limit
 * is reached, the data word address will "roll over" and sequential read
 * will continue. The sequential read operation is terminated when the 
 * microcontroller does not respond when a zero but does generate a
 * following stop condition.
 */
static int lm75a_2bytes_read(int fd, unsigned char addr,
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
	msgs[1].len	= 2;
	msgs[1].buf	= buf;

	msgset.msgs	= msgs;
	msgset.nmsgs	= 2;

	rc = ioctl(fd, I2C_RDWR, &msgset);
	if (rc != 2)
		printf("Loss packet %d on Sequential Read.\n", rc);
	return rc;
}

/* Presetn Pointer Read
 * 
 *           S
 *           T               R                                   S
 *           A               E                                   T
 *           R               A                                   O
 *           T               D                                   P
 *          +-+-+ +-+ +-+-+-+-+ +-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+-+
 *          | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 * SDA LINE | | | | | |     | | |             | |             | | |
 *          | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 *          +-+ +-+ +-+-+-+-+ +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ +-+
 *             M           L R A       MSB     A    LSB        N
 *             S           S / C               C               O
 *             B           B W K               K
 *                                                             A
 *                                                             C
 *                                                             K
 *
 * The internal data word address counter maintains the last address
 * accessed during the last read or write operation, incremented by one,
 * This address stays valid between operations as long as the chip power
 * is maintained. The address "roll over" during read is from the last
 * of the last memory page to the first byte of the first page. The
 * address "roll over" during write is from the last byte of the current
 * page to the first byte of the same page.
 *
 * Once the device address with the read/write select bit set to one is
 * clocked in and acknowledged by the EEPROM, the current address data
 * word is serially clocked out. The microcontroller does not respond
 * with an input zero but does generated a following stop condition.
 *
 */
static int lm75a_present_read(int fd, unsigned char addr, unsigned char *buf)
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
		printf("Loss packet %d on Current Address Read\n", rc);
	return rc;
}

/* Configuration Register Write
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
static int lm75a_write(int fd, unsigned char addr, unsigned char offset, 
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

/* Tos or Thyst Register Write
 * 
 * SDA LINE
 *
 *
 *
 *  S               W
 *  T               R                                         S
 *  A               I                                         T
 *  R  DEVICE       T                                         O
 *  T ADDRESS       E    WORD ADDRESS       MSB       LSB     P
 * +-+-+ +-+ +-+-+-+ + +-+-+-+-+-+-+-+-+ +-+-+-+-+ +-+-+-+-+ +-+
 * | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 * | | | | | |     | | |*              | |  ...  | |  ...  | | |
 * | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 * +-+ +-+ +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    M           L R A   M           L A         A         A
 *    S           S / C   S           S C         C         C
 *    B           B W K   B           B K         K         K
 *
 *
 * The 1K/2K EEPROM is capable of an 8-byte page write, and the 4K,
 * 8K and 16K devices are capable of 16-byte page writes.
 *
 * A page write is initiated the same as a byte write, but the
 * microcontroller does not send stop condition after the first data 
 * word is clocked in. Instead, after the EEPROM acknowledges receipt
 * of the first data word, the microcontroller can transmit up to seven
 * (1K/2K) or fifteen (4K,8K,16K) more data words. The EEPROM will
 * respond with a zero after each data word received. The microcontroller
 * must terminate the page write sequence with a stop condition.
 *
 * The data wrod address lower three (1K/2K) or four (4K,8K,16K) bits are
 * internally incremented following the receipt of each data word. The
 * higher data word address bits are not incremented, retaining the memory
 * page row location. When the word address, internally generated, reaches
 * the page boundary, the following byte is placed at the begining of the
 * same page. If more then eight (1K/2K) or sixteen (4K/8K/16K) data words
 * are transmitted to the EEPROM, the data word address will "roll over"
 * and previous data will overwritten. 
 *
 */
static int lm75a_2bytes_write(int fd, unsigned char addr, unsigned char offset,
						unsigned char *buf)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs;
	unsigned char *tmp;
	int rc;

	/* malloc */
	tmp = malloc(3);
	tmp[0] = offset;
	memcpy(&tmp[1], buf, 2);

	msgs.addr	= addr & 0xfe;
	msgs.flags	= I2C_M_WR;
	msgs.len	= 3;
	msgs.buf	= tmp;

	msgset.msgs	= &msgs;
	msgset.nmsgs	= 1;

	rc = ioctl(fd, I2C_RDWR, &msgset);
	if (rc != 1)
		printf("Loss packet %d on Page write\n", rc);
	free(tmp);
	return rc;
}

int main()
{
	short value;
	char buf[2];
	int fd, ret;
	int idx;

	/* Open I2C Bus */
	fd = open(I2C_BUS, O_RDWR);
	if (fd < 0) {
		printf("Unable to open %s\n", I2C_BUS);
		return -1;
	}

	memset(buf, 2, 0);
	/* Setup LM75A normal mode  */
	lm75a_write(fd, I2C_ADDR, LM75A_CONFIG_REG, 0x00);

	/* Temperature register (Temp) read */
	lm75a_2bytes_read(fd, I2C_ADDR, LM75A_TEMPER_REG, buf);
	value  = (buf[0] & TEMP_MSB_MASK) << 3;
	value |= (buf[1] >> 5) & TEMP_LSB_MASK;
	printf("Temperature: %f C\n", (float)value * 0.125);

	/* Close I2C Bus */
	close(fd);

	return 0;
}
