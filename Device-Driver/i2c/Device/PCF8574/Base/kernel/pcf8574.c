/*
 * PCF8574 IO Device Driver
 *
 * (C) 2019.10.30 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>

/* pcf8574 on DTS
 *
 * arch/arm/boot/dts/bcm2711-rpi-4-b.dts
 *
 * &i2c1 {
 *        pcf8574@38 {
 *               compatible = "BiscuitOS,pcf8574";
 *               reg = <0x38>;
 *               BD-gpio = <&gpio 25 GPIO_ACTIVE_HIGH>;
 *        };
 * };
 *
 * The Interrupt is GPIO25 which IRQ number is 58, the pin
 * as figure:
 *
 * PaspberryPi 4B GPIO
 *
 *                    +---------+
 *                3V3 | 1     2 | 5V
 *       (SDA1) GPIO2 | 3     4 | 5V
 *       (SCL1) GPIO3 | 5     6 | GND
 *  (GPIO_GCLK) GPIO4 | 7     8 | GPIO14 (TXD0)
 *                GND | 9    10 | GPIO15 (RXD0)
 * (GPIO_GEN0) GPIO17 | 11   12 | GPIO18 (GPIO_GEN1)
 * (GPIO_GEN2) GPIO27 | 13   14 | GND
 * (GPIO_GEN3) GPIO22 | 15   16 | GPIO23 (GPIO_GEN4)
 *               3V3  | 17   18 | GPIO24 (GPIO_GEN5)
 *  (SPI_MOSI) GPIO10 | 19   20 | GND
 *   (SPI_MISO) GPIO9 | 21   22 | GPIO25 (GPIO_GEN6) <------ Interrupt
 *  (SPI_SCLK) GPIO11 | 23   24 | GPIO8  (SPI_CE0_N)
 *                GND | 25   26 | GPIO7  (SPI_CE1_N)
 *              ID_SD | 27   28 | ID_SC
 *              GPIO5 | 29   30 | GND
 *              GPIO6 | 31   32 | GPIO12
 *             GPIO13 | 33   34 | GND
 *             GPIO19 | 35   36 | GPIO16
 *             GPIO26 | 37   38 | GPIO20
 *                GND | 39   40 | GPIO21
 *                    +---------+
 *
 */

/* I2C Device Name */
#define DEV_NAME		"pcf8574"
#define SLAVE_I2C_ADDR		0x38
#define I2C_M_WR		0x00

/* GPIO mapping */
#define GPIO1			0x01
#define GPIO2			0x02
#define GPIO3			0x04
#define GPIO4			0x08
#define GPIO5			0x10
#define GPIO6			0x20
#define GPIO7			0x40
#define GPIO8			0x80
#define GPIO_ALL		0xFF

#define __unused		__attribute__((unused))

struct pcf8574_priv
{
	struct i2c_client *client;
	struct work_struct wq;
	int irq;
	int gpio;
	u8  data; /* priv gpio state */
};

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
static int __unused pcf8574a_read(struct i2c_client *client, 
				unsigned char offset, unsigned char *buf) 
{
	struct i2c_msg msgs;
	int ret;

	msgs.addr	= client->addr;
	msgs.flags	= I2C_M_RD;
	msgs.len	= 2;
	msgs.buf	= buf;

	ret = i2c_transfer(client->adapter, &msgs, 1);
	if (1 != ret)
		printk(KERN_ERR "Loss packet %d on Random Read\n", ret);
	return ret;
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
static int __unused pcf8574a_write(struct i2c_client *client, 
				unsigned char offset, unsigned char data)
{
	struct i2c_msg msgs;
	unsigned char tmp[2];
	int ret;

	tmp[0]		= offset;
	tmp[1]		= data;
	msgs.addr	= client->addr;
	msgs.flags	= I2C_M_WR;
	msgs.len	= 2;
	msgs.buf	= tmp;

	ret = i2c_transfer(client->adapter, &msgs, 1);
	if (1 != ret)
		printk("Loss packet %d on Byte write\n", ret);
	return ret;
}

static void wq_isr(struct work_struct *work)
{
	struct pcf8574_priv *priv;
	unsigned char buf[2];

	priv = container_of(work, struct pcf8574_priv, wq);
	/* Read GPIO Status */
	memset(buf, 0, 2);
	pcf8574a_read(priv->client, GPIO_ALL, buf);
	printk("GPIO Status: %#hhx\n", buf[0]);
}

/* IRQ handler */
static irqreturn_t pcf8574_isr(int irq, void *dev_id)
{
	struct pcf8574_priv *priv = (struct pcf8574_priv *)dev_id;
	
	/* Low speed deal with on work queue */
	schedule_work(&priv->wq);

	return IRQ_HANDLED;
}

/* Probe: (LDD) Initialize Device */
static int pcf8574_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct device_node *np = client->dev.of_node;
	struct pcf8574_priv *priv;
	int ret;

	/* allocate memory */
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		printk("ERROR: no free memory\n");
		ret = -ENOMEM;
		goto err_alloc;
	}
	
	/* Get IRQ number from DTB */
	priv->gpio = of_get_named_gpio(np, "BD-gpio", 0);
	if (priv->gpio < 0) {
		printk("Error: unable to get GPIO from DTS\n");
		ret = -EINVAL;
		goto err_gpio;
	}
	gpio_direction_input(priv->gpio);
	priv->irq = gpio_to_irq(priv->gpio);
	if (priv->irq < 0) {
		printk("Error: unable to get IRQ\n");
		ret = -EINVAL;
		goto err_gpio;
	}

	priv->client = client;
	i2c_set_clientdata(client, priv);
	/* setup GPIO3 as down */
	pcf8574a_write(client, GPIO_ALL, GPIO3);
	/* Initialize workqueue */
	INIT_WORK(&priv->wq, wq_isr);
	mdelay(100);

	/* Require IRQ */
	ret = request_irq(priv->irq, pcf8574_isr,
				IRQF_TRIGGER_RISING, 
				DEV_NAME, priv);
	if (ret < 0) {
		printk("Can't request IRQ %d\n", priv->irq);
		return -EINVAL;
		goto err_gpio;
	}

	printk("GPIO-Export IRQ: %d\n", priv->irq);

	return 0;

err_gpio:
	kfree(priv);
err_alloc:
	return ret;
}

/* Remove: (LDD) Remove Device (Module) */
static int pcf8574_remove(struct i2c_client *client)
{
	struct pcf8574_priv *priv = dev_get_drvdata(&client->dev);

	/* Free IRQ */
	free_irq(priv->irq, priv);
	kfree(priv);
	i2c_set_clientdata(client, NULL);

	return 0;
}

static struct of_device_id pcf8574_match_table[] = {
	{ .compatible = "BiscuitOS,pcf8574", },
	{ },
};

static const struct i2c_device_id pcf8574_id[] = {
	{ DEV_NAME, SLAVE_I2C_ADDR },
	{ },
};

static struct i2c_driver pcf8574_driver = {
	.driver = {
		.name = DEV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = pcf8574_match_table,
	},
	.probe	= pcf8574_probe,
	.remove	= pcf8574_remove,
	.id_table = pcf8574_id,
};

module_i2c_driver(pcf8574_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("PCF8574 IO Device Driver Module");
