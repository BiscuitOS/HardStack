SPI on Kernel
-------------------------------------

SPI drive on kernel see `spi.c`. Usage of driver as follow:

```
make
sudo insmod spi.ko
```
### SPI Slave device on DTS

On newest mainstream linux, the SPI device information describe on DTS, 
Here offer the way to describe on DTS.

``
&spi0 {
    status = "okay";
    
    spi_demo@2 {
        compatible = "spi_demo";
        #address-cells = <1>;
        #size-cells = <1>;
        reg = <0x2>;
        spi-max-frequency = <5000000>;
    };
};
```
The reg describe CS for I2C slave device, and compatible must be same on 
driver code.

### SPI Transfer

The core layout for SPI read and write, the implement as follow:

```
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
```

### SPI Read

The SPI read base on `spi_sync`, details as follow:

```
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

```

### SPI write

The SPI write base on `spi_sync`, details as follow:

```
static void spi_demo_write_reg(struct spi_device *spi, u8 reg, uint8_t val)
{
    struct spi_demo_priv *priv = spi_get_drvdata(spi);

    priv->spi_tx_buf[0] = INSTRUCTION_WRITE;
    priv->spi_tx_buf[1] = reg;
    priv->spi_tx_buf[2] = val;

    spi_demo_trans(spi, 3);
}
```
