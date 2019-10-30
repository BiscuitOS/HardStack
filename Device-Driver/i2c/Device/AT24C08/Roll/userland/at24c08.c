/*
 * AT24C08 Userland
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
#include <malloc.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define I2C_BUS		"/dev/i2c-1"
#define I2C_ADDR	0x50
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

/* Squential Read
 *
 * SDA LINE
 *
 *
 *          R                                                   S
 *          E             A           A           A             T
 * DEVICE   A             C           C           C             O
 * ADDRESS  D             K           K           K             P
 * - - - - +-+ +-+-+-+-+-+ +-+-+-+-+-+ +-+-+-+-+-+ +-+-+-+-+-+-+-+
 *       | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 *         | | |  ...  | | |  ...  | | |  ...  | | |  ...  | | | |
 *       | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 * - - - - + +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ +-+
 *          R A  Data n      Data n+1    Data n+2    Data n+3 N
 *          / C                                               O
 *          W K
 *                                                            A
 *                                                            C
 *                                                            K
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
static int Sequen_read(int fd, unsigned char addr,
			unsigned char offset, unsigned char *buf, int len)
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
	msgs[1].len	= len;
	msgs[1].buf	= buf;

	msgset.msgs	= msgs;
	msgset.nmsgs	= 2;

	rc = ioctl(fd, I2C_RDWR, &msgset);
	if (rc != 2)
		printf("Loss packet %d on Sequential Read.\n", rc);
	return rc;
}

/* Current Address Read
 * 
 *           S
 *           T               R                     S
 *           A               E                     T
 *           R               A                     O
 *           T               D                     P
 *          +-+-+ +-+ +-+-+-+-+ +-+-+-+-+-+-+-+-+-+-+
 *          | | | | | | | | | | | | | | | | | | | | |
 * SDA LINE | | | | | |     | | |               | | |
 *          | | | | | | | | | | | | | | | | | | | | |
 *          +-+ +-+ +-+-+-+-+ +-+-+-+-+-+-+-+-+-+ +-+
 *             M           L R A      DATA       N
 *             S           S / C                 O
 *             B           B W K
 *                                               A
 *                                               C
 *                                               K
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
static int Current_Address_read(int fd, unsigned char addr, unsigned char *buf)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs;
	int rc;

	msgs.addr	= addr & 0xfe;
	msgs.flags	= I2C_M_RD;
	msgs.len	= 1;
	msgs.buf	= buf;

	msgset.msgs	= &msgs;
	msgset.nmsgs	= 1;

	rc = ioctl(fd, I2C_RDWR, &msgset);
	if (rc != 1)
		printf("Loss packet %d on Current Address Read\n", rc);
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

/* Page Write
 * 
 * SDA LINE
 *
 *
 *
 *  S               W
 *  T               R                                                   S
 *  A               I                                                   T
 *  R  DEVICE       T                                                   O
 *  T ADDRESS       E    WORD ADDRESS      DATAn    DATAm     DATAp     P
 * +-+-+ +-+ +-+-+-+ + +-+-+-+-+-+-+-+-+ +-+-+-+-+ +-+-+-+-+ +-+-+-+-+ +-+
 * | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 * | | | | | |     | | |*              | |  ...  | |  ...  | |  ...  | | |
 * | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 * +-+ +-+ +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    M           L R A   M           L A         A         A         A
 *    S           S / C   S           S C         C         C         C
 *    B           B W K   B           B K         K         K         K
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
static int Page_write(int fd, unsigned char addr, unsigned char offset,
				unsigned char *buf, int len)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs;
	unsigned char *tmp;
	int rc;

	/* malloc */
	tmp = malloc(len + 1);
	tmp[0] = offset;
	memcpy(&tmp[1], buf, len);

	msgs.addr	= addr & 0xfe;
	msgs.flags	= I2C_M_WR;
	msgs.len	= len + 1;
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
	buf[0] = 0x68;
	ret = Byte_write(fd, I2C_ADDR, 0x00, buf[0]);
	if (ret < 0) {
		printf("I2C Byte Write error\n");
		close(fd);
		return -1;
	}
	/* need delay to wait I2C bus */
	usleep(10000);

	/* Page Write */
	memset(buf, 0, 8);
	buf[0] = 0x21;
	buf[1] = 0x22;
	buf[2] = 0x23;
	buf[3] = 0x24;
	buf[4] = 0x25;
	ret = Page_write(fd, I2C_ADDR, 0xFE, buf, 5);
	if (ret < 0) {
		printf("I2C Page Write error\n");
		close(fd);
		return -1;
	}
	/* need delay before read */
	usleep(10000);

	/********** Read Operation **********/
	
	/* Random Read  */
	memset(buf, 0, 8);
	ret = Random_read(fd, I2C_ADDR, 0x00, buf);
	if (ret < 0) {
		printf("I2C Random Read error.\n");
		close(fd);
		return -1;
	}
	printf("Random Read 0x0 DATA: %#x\n", buf[0]);

	/* Sequential Read */
	memset(buf, 0, 8);
	ret = Sequen_read(fd, I2C_ADDR, 0xFE, buf, 5);
	if (ret < 0) {
		printf("I2C Sequential Read error.\n");
		close(fd);
		return -1;
	}
	for (idx = 0; idx < 5; idx++)
		printf("Buf[%d] %#x\n", idx, buf[idx]);

	/* Close I2C Bus */
	close(fd);

	return 0;
}
