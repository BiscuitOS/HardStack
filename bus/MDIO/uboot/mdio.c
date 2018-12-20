/*
 * MDIO (SMI/MIIM) read/write on Uboot
 *
 * (C) 2018.11.17 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <common.h>
#include <miiphy.h>

/*
 * MDIO read
 *  @phy: PHY ID
 *  @reg: Register address.
 *  @data: read buffer.
 *
 * If succeed, return 0.
 */
static int mdio_read(unsigned char phy, unsigned char reg, 
                                             unsigned short *data)
{
    const char *devname;

    /* use current device */
    devname = miiphy_get_current_dev();

    if (miiphy_read(devname, phy, reg, &data) != 0) {
        printf("Error reading from the PHY %d reg %d\n", phy, reg);
        return -1;
    }
    return 0;
}

/*
 * MDIO write
 *  @phy: PHY ID
 *  @reg: Register address.
 *  @data: data need to write.
 */
static int mdio_write(unsigned char phy, unsigned char reg,
                                              unsigned short data)
{
    const char *devname;

    /* use current device */
    devname = miiphy_get_current_dev();

    if (miiphy_write(devname, phy, reg, data) != 0) {
        printf("Error writing to the PHY %d reg %d\n", phy, reg);
        return -1;
    }
    return 0;
}
