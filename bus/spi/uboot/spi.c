/*
 * SPI read/write on Uboot
 *
 * (C) 2018.11.18 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <common.h>
#include <spi.h>

/* Init slave device */
struct spi_slave *spi_slave_init(void)
{
    struct spi_slave *slave;
    unsigned int bus = 0;
    unsigned int cs  = 2;
    unsigned int mode = SPI_MODE_0;

    slave = spi_setup_slave(bus, cs, 1000000, mode);
    if (!slave) {
        printf("Invalid device %d:%d\n", bus, cs);
        return NULL;
    }

    spi_claim_bus(slave);

    return slave;
}

/* SPI read/write operation */
static int spi_read_write(struct spi_slave *spi, const u8 *cmd, 
                    size_t cmd_len, const u8 *data_out, size_t data_len)
{
    unsigned long flags = SPI_XFER_BEGIN;
    int ret;

    if (data_len == 0)
        flags |= SPI_XFER_END;

    ret = spi_xfer(spi, cmd_len * 8, cmd, NULL, flags);
    if (ret) {
        printf("Invalid send command\n");
    } else if (data_len != 0) {
        ret = spi_xfer(spi, data_len * 8, data_out, data_on, SPI_XFER_END);
        if (ret)
            printf("Incalid transfer data\n");
    }

    return ret;
}
