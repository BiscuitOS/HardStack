/*
 * AT24C08 Device Driver
 *
 * (C) 2019.10.22 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

/* at24c08 on DTS
 *
 * arch/arm/boot/dts/bcm2711-rpi-4-b.dts
 *
 * &i2c1 {
 *        at24c08@50 {
 *               compatible = "BiscuitOS,at24c08";
 *               reg = <0x50>;
 *        };
 * };
 */

/* I2C Device Name */
#define DEV_NAME	"at24c08"
#define SLAVE_I2C_ADDR	0x50

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
static int Random_read(struct i2c_client *client, 
				unsigned char offset, unsigned char *buf) 
{
	struct i2c_msg msgs[2];
	int ret;

	msgs[0].addr	= client->addr;
	msgs[0].flags	= client->flags & I2C_M_TEN;
	msgs[0].len	= 1;
	msgs[0].buf	= &offset;

	msgs[1].addr	= client->addr;
	msgs[1].flags	= I2C_M_RD;
	msgs[1].len	= 1;
	msgs[1].buf	= buf;

	ret = i2c_transfer(client->adapter, msgs, 2);
	if (2 != ret)
		printk(KERN_ERR "Loss packet %d on Random Read\n", ret);
	return ret;
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
static int Sequen_read(struct i2c_client *client, unsigned char offset,
			unsigned char *buf, int len)
{
	struct i2c_msg msgs[2];
	int ret;

	msgs[0].addr	= client->addr;
	msgs[0].flags	= client->flags & I2C_M_TEN;
	msgs[0].len	= 1;
	msgs[0].buf	= &offset;

	msgs[1].addr	= client->addr;
	msgs[1].flags	= I2C_M_RD;
	msgs[1].len	= len;
	msgs[1].buf	= buf;

	ret = i2c_transfer(client->adapter, msgs, 2);
	if (2 != ret)
		printk(KERN_ERR "Loss packet %d on Sequen Read\n", ret);
	return ret;
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
static int Current_Address_read(struct i2c_client *client, unsigned char *buf)
{
	struct i2c_msg msgs;
	int ret;

	msgs.addr	= client->addr;
	msgs.flags	= I2C_M_RD;
	msgs.len	= 1;
	msgs.buf	= buf;

	ret = i2c_transfer(client->adapter, &msgs, 1);
	if (1 != ret)
		printk("Loss packet %d on Current Address Read\n", ret);
	return ret;
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
static int Byte_write(struct i2c_client *client, unsigned char offset,
							unsigned char data)
{
	struct i2c_msg msgs;
	unsigned char tmp[2];
	int ret;

	tmp[0]		= offset;
	tmp[1]		= data;
	msgs.addr	= client->addr;
	msgs.flags	= client->flags;
	msgs.len	= 2;
	msgs.buf	= tmp;

	ret = i2c_transfer(client->adapter, &msgs, 1);
	if (1 != ret)
		printk("Loss packet %d on Byte write\n", ret);
	return ret;
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
static int Page_write(struct i2c_client *client, unsigned char offset,
						unsigned char *buf, int len)
{
	struct i2c_msg msgs;
	unsigned char *tmp;
	int ret;

	/* kzalloc */
	tmp = kzalloc(len + 1, GFP_KERNEL);
	if (!tmp) {
		printk("Unable to allocate memory\n");
		return -ENOMEM;
	}

	tmp[0] = offset;
	memcpy(&tmp[1], buf, len);

	msgs.addr	= client->addr;
	msgs.flags	= client->flags;
	msgs.len	= len + 1;
	msgs.buf	= tmp;

	ret = i2c_transfer(client->adapter, &msgs, 1);
	if (1 != ret)
		printk("Loss packet %d on Page write\n", ret);
	kfree(tmp);
	return ret;
}

/* Probe: (LDD) Initialize Device */
static int at24c08_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	char buf[8];
	int ret, idx;
	
	/*********** Write Operations **************/

	/* Byte write */
	memset(buf, 0, 8);
	buf[0] = 0x88;
	ret = Byte_write(client, 0x00, buf[0]);
	if (ret < 0) {
		printk("I2C Byte Write error\n");
		return -EIO;
	}
	/* need delay to wait I2C bus */
	mdelay(10);

	/* Page Write */
	memset(buf, 0, 8);
	buf[0] = 0xFF;
	buf[1] = 0x11;
	buf[2] = 0x22;
	buf[3] = 0x33;
	buf[4] = 0x44;
	ret = Page_write(client, 0x01, buf, 5);
	if (ret < 0) {
		printk("I2C Page Write error\n");
		return -EIO;
	}
	mdelay(10);

	/*********** Read Operations **************/

	/* Random Read */
	memset(buf, 0, 8);
	ret = Random_read(client, 0x00, buf);
	if (ret < 0) {
		printk(KERN_ERR "I2C Random Read error.\n");
		return -EIO;
	}
	printk("Random Read 0x00 DATA:     %#x\n", buf[0]);

	/* Current Address Read */
	memset(buf, 0, 8);
	ret = Current_Address_read(client, buf);
	if (ret < 0) {
		printk("I2C Current Address Read error\n");
	}
	printk("Current Address Read Data: %#x\n", buf[0]);

	/* Sequential Read */
	memset(buf, 0, 8);
	ret = Sequen_read(client, 0x01, buf, 5);
	if (ret < 0) {
		printk(KERN_ERR "I2C Sequential Read error.\n");
		return -EIO;
	}
	for (idx = 0; idx < 5; idx++)
		printk("Buf[%d] %#x\n", idx, buf[idx]);

	return 0;
}

/* Remove: (LDD) Remove Device (Module) */
static int at24c08_remove(struct i2c_client *client)
{
	return 0;
}

static struct of_device_id at24c08_match_table[] = {
	{ .compatible = "BiscuitOS,at24c08", },
	{ },
};

static const struct i2c_device_id at24c08_id[] = {
	{ DEV_NAME, SLAVE_I2C_ADDR },
	{ },
};

static struct i2c_driver at24c08_driver = {
	.driver = {
		.name = DEV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = at24c08_match_table,
	},
	.probe	= at24c08_probe,
	.remove	= at24c08_remove,
	.id_table = at24c08_id,
};

module_i2c_driver(at24c08_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("AT24C08 EEPROM Device Driver Module");
