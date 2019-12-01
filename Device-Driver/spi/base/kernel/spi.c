/*
 * SPI device driver
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Private DTS file: DTS_demo.dtsi
 *
 * / {
 *        spi_demo {
 *                compatible = "spi_demo, BiscuitOS";
 *                status = "okay";
 *        };
 * };
 *
 * On Core dtsi:
 *
https://blog.csdn.net/u013656962/article/details/81179419
 * include "DTS_demo.dtsi"
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>

/* LDD Platform Name */
#define DEV_NAME		"spi_demo"
#define SPI_TRANSFER_BUF_LEN	32
#define INSTRUCTION_WRITE	0x02
#define INSTRUCTION_READ	0x03

struct spi_demo_dev
{
	struct spi_device *spi;
	u8 *spi_tx_buf;
	u8 *spi_rx_buf;
};

static int spi_demo_read(struct spi_device *spi, uint8_t addr, 
							u8 *buf, int len)
{
	struct spi_demo_dev *sdev = spi_get_drvdata(spi);
	struct spi_message msg;
	struct spi_transfer t;
	int ret;

	sdev->spi_tx_buf[0] = INSTRUCTION_READ;
	sdev->spi_tx_buf[1] = addr;

	/* Transfer information */
	t.tx_buf    = sdev->spi_tx_buf;
	t.rx_buf    = sdev->spi_rx_buf;
	t.len       = len + 2;
	t.cs_change = 0;

	/* Configure spi message */
	spi_message_init(&msg);
	spi_message_add_tail(&t, &msg);

	ret = spi_sync(spi, &msg);
	if (ret) {
		printk("%s spi transfer failed: %d\n", __func__, ret);
		
	}

	/* Read data from RX buffer */
	strncpy(buf, &sdev->spi_rx_buf[2], len);
	return ret;
}

/* Probe: (LDD) Initialize Device */
static int spi_demo_probe(struct spi_device *spi)
{
	struct spi_demo_dev sdev;
	int ret;
	u8 buf;

	/* Allocate SPI TX Buffer */
	sdev.spi_tx_buf = kzalloc(SPI_TRANSFER_BUF_LEN, GFP_KERNEL);
	if (!sdev.spi_tx_buf) {
		printk("TXBUF: No Free Memory.\n");
		ret = -ENOMEM;
		goto err_tx;
	}
	/* Allocate SPI RX Buffer */
	sdev.spi_rx_buf = kzalloc(SPI_TRANSFER_BUF_LEN, GFP_KERNEL);
	if (!sdev.spi_rx_buf) {
		printk("RXBUF: No Free Memory.\n");
		ret = -ENOMEM;
		goto err_rx;
	}
	sdev.spi = spi;
	spi_set_drvdata(spi, &sdev);

	/* Setup SPI */
	spi->mode = SPI_MODE_3;
	spi->bits_per_word = 8;
	spi_setup(spi);

	/* Read Register */
	spi_demo_read(spi, 0x00, &buf, 1);
	printk("Buf: %#hhx\n", buf);

	return 0;

err_rx:
	kfree(sdev.spi_tx_buf);
err_tx:
	return ret;
}

/* Remove: (LDD) Remove Device (Module) */
static int spi_demo_remove(struct spi_device *spi)
{
	struct spi_demo_dev *sdev = spi_get_drvdata(spi);

	kfree(sdev->spi_rx_buf);
	kfree(sdev->spi_tx_buf);
	kfree(sdev);

	return 0;
}

static const struct of_device_id spi_demo_of_match[] = {
	{ .compatible = "BiscuitOS,spi_demo", },
	{ },
};

static const struct spi_device_id spi_demo_id[] = {
	{ DEV_NAME },
	{ }
};
MODULE_DEVICE_TABLE(spi, spi_demo_id);

/* Platform Driver Information */
static struct spi_driver spi_demo_driver = {
	.probe    = spi_demo_probe,
	.remove   = spi_demo_remove,
	.id_table = spi_demo_id,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
		.of_match_table	= of_match_ptr(spi_demo_of_match),
	},
};
module_spi_driver(spi_demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("SPI Device Driver with DTS");
