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
 * include "DTS_demo.dtsi"
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>

/* DDL Platform Name */
#define DEV_NAME		"spi_demo"
#define SPI_TRANSFER_BUF_LEN	32

struct spi_demo_priv {
	struct spi_device *spi;
	u8 *spi_tx_buf;
	u8 *spi_rx_buf;
};

/* Probe: (DDL) Initialize Device */
static int spi_demo_probe(struct spi_device *spi)
{
	struct spi_demo_priv *pdata;
	int ret;

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (pdata) {
		ret = -ENOMEM;
		goto err_alloc;
	}

	/* Allocate non-DMA buffers */
	pdata->spi_tx_buf = kzalloc(SPI_TRANSFER_BUF_LEN, GFP_KERNEL);
	if (!pdata->spi_tx_buf) {
		ret = -ENOMEM;
		goto err_tx;
	}
	pdata->spi_rx_buf = kzalloc(SPI_TRANSFER_BUF_LEN, GFP_KERNEL);
	if (!pdata->spi_rx_buf) {
		ret = -ENOMEM;
		goto err_rx;
	}
	
	spi_set_drvdata(spi, &pdata);
	/* Configure the SPI bus */
	spi->bits_per_word = 8;
	spi->max_speed_hz = spi->max_speed_hz ? : 10 * 1000 * 1000;

	/* Power up SPI slave device */
	spi_setup(spi);

	return 0;

err_rx:
	kfree(pdata->spi_tx_buf);
err_tx:
	kfree(pdata);
err_alloc:
	return ret;
}

/* Remove: (DDL) Remove Device (Module) */
static int spi_demo_remove(struct spi_device *spi)
{
	return 0;
}

static const struct of_device_id spi_demo_of_match[] = {
	{ .compatible = "BiscuitOS,spi", },
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
MODULE_DESCRIPTION("Platform Device Driver with DTS");
