/*
 * SPI read/write on Kernel
 *
 * (C) 2018.11.19 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <linux/of.h>
#include <linux/of_device.h>

/*
 * SPI device on DTS
 *
 * &spi0 {
 *     status = "okay";
 *     
 *     spi_demo@2 {
 *         compatible = "spi_demo";
 *         #address-cells = <1>;
 *         #size-cells = <1>;
 *         reg = <0x2>;
 *         spi-max-frequency = <5000000>;
 *     };
 * };
 */

#define DEV_NAME                "spi_demo"
#define SPI_TRANSFER_BUF_LEN    14

/* SPI interface instruction set */
#define INSTRUCTION_WRITE       0x02
#define INSTRUCTION_READ        0x03
#define INSTRUCTION_BIT_MODIFY  0x05

struct spi_demo_priv {
    struct spi_device *spi;

    u8 *spi_tx_buf;
    u8 *spi_rx_buf;
};

static int spi_demo_trans(struct spi_device *spi, int len)
{
    struct spi_demo_priv *priv = spi_get_drvdata(spi);
    struct spi_transfer t = {
        .tx_buf = priv->spi_tx_buf,
        .rx_buf = priv->spi_rx_buf,
        .len    = len,
        .cs_change = 0,
    };
    struct spi_message msg;
    int ret;

    spi_message_init(&msg);

    spi_message_add_tail(&t, &msg);

    ret = spi_sync(spi, &msg);
    if (ret)
        printk(KERN_ERR "spi transfer failed, ret = %d\n", ret);
    return ret;
}

/* 
 * Read one byte from SPI slave device 
 *  @spi: spi slave device
 *  @reg: register need to read.
 *
 * if succeed, return the value of register.
 */
static u8 spi_demo_read_reg(struct spi_device *spi, uint8_t reg)
{
    struct spi_demo_priv *priv = spi_get_drvdata(spi);
    u8 val = 0;

    priv->spi_tx_buf[0] = INSTRUCTION_READ;
    priv->spi_tx_buf[1] = reg;

    spi_demo_trans(spi, 3);
    val = priv->spi_rx_buf[2];

    return val;
}

/*
 * Read two byte from SPI slave device 
 *  @spi: SPi slave device.
 *  @reg: register need to read.
 *
 * if succeed, v1 is first registe's value and v2 is second register's value.
 */
static void spi_demo_read_2regs(struct spi_device *spi, uint8_t reg,
                            uint8_t *v1, uint8_t *v2)
{
    struct spi_demo_priv *priv = spi_get_drvdata(spi);

    priv->spi_tx_buf[0] = INSTRUCTION_READ;
    priv->spi_tx_buf[1] = reg;

    spi_demo_trans(spi, 4);

    *v1 = priv->spi_rx_buf[2];
    *v2 = priv->spi_rx_buf[3];
}

/*
 * Write one byte to SPI slave device
 *  @spi: SPI slave device
 *  @reg: Regsiter need to write
 *  @val: data need to write.
 */
static void spi_demo_write_reg(struct spi_device *spi, u8 reg, uint8_t val)
{
    struct spi_demo_priv *priv = spi_get_drvdata(spi);

    priv->spi_tx_buf[0] = INSTRUCTION_WRITE;
    priv->spi_tx_buf[1] = reg;
    priv->spi_tx_buf[2] = val;

    spi_demo_trans(spi, 3);
}

/*
 * Change speical bit on SPI slave device.
 *  @spi: SPI slave device.
 *  @reg: register need to write.
 *  @mask: mask for modify.
 *  @val: data need to write.
 */
static void spi_demo_write_bits(struct spi_device *spi, u8 reg, 
                                   u8 mask, uint8_t val)
{
    struct spi_demo_priv *priv = spi_get_drvdata(spi);

    priv->spi_tx_buf[0] = INSTRUCTION_BIT_MODIFY;
    priv->spi_tx_buf[1] = reg;
    priv->spi_tx_buf[2] = mask;
    priv->spi_tx_buf[3] = val;

    spi_demo_trans(spi, 4);
}

/*
 * SPI Slave device probe entence
 */
static int spi_demo_probe(struct spi_device *spi)
{
    struct spi_demo_priv priv;
    int value;

    spi_set_drvdata(spi, &priv);

    /* Configure the SPI bus */
    spi->bits_per_word = 8;
    spi->max_speed_hz = spi->max_speed_hz ? : 10 * 1000 * 1000;

    /* Power up SPI slave device */

    spi_setup(spi);

    /* Allocate non-DMA buffers */
    priv.spi_tx_buf = devm_kzalloc(&spi->dev, SPI_TRANSFER_BUF_LEN, 
                                          GFP_KERNEL);
    priv.spi_rx_buf = devm_kzalloc(&spi->dev, SPI_TRANSFER_BUF_LEN,
                                          GFP_KERNEL);

    /* Read data from SPI slave device */
    value = spi_demo_read_reg(spi, 0x10);

    /* Write data into SPI slave device */
    spi_demo_write_reg(spi, 0x10, value);

    return 0;
}

/*
 * SPI Slave device remove entence
 */
static int spi_demo_remove(struct spi_device *spi)
{
    /* Power off SPI slave device */

    return 0;
}

static const struct of_device_id spi_demo_dt_ids[] = {
    { .compatible = DEV_NAME },
    { },
};

static const struct spi_device_id spi_demo_id[] = {
    { DEV_NAME },
    { }
};

MODULE_DEVICE_TABLE(spi, spi_demo_id);

static struct spi_driver spi_demo_driver = {
    .driver = {
        .name  = DEV_NAME,
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(spi_demo_dt_ids),
    },
    .probe    = spi_demo_probe,
    .remove   = spi_demo_remove,
    .id_table = spi_demo_id,
};

module_spi_driver(spi_demo_driver);

MODULE_LICENSE("GPL v2");
MODULE_ALIAS("spi demo");
