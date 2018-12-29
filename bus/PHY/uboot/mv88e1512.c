/*
 * Marvell MV88E1512 PHY driver on uboot
 *
 * (C) 2018.12.27 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <common.h>
#include <phy.h>

#define DEV_NAME             "phy_demo"
#define PHY_DEMO_ID          0x1410dd0
#define PHY_DEMO_ID_MASK     0xffffff0

#define PHY_DEMO_PHY_PAGE    0x16
#define PHY_DEMO_GENCTR      0x14
#define PHY_DEMO_MODE_SGMII  0x01

#define PHY_DEMO_PHY_EXT_SR         0x1b
#define PHY_DEMO_HWCFG_MODE_MASK    0x0f
#define PHY_DEMO_HWCFG_MODE_SGMII_NO_CLK     0x4
#define PHY_DEMO_HWCFG_FIBER_COPPER_AUTO     0x8000
#define PHY_AUTONEGOTIATE_TIMEOUT            5000

#define PHY_DEMO_PHY_STATUS         0x11
#define PHY_DEMO_PHYSTAT_LINK       0x0400
#define PHY_DEMO_PHYSTAT_SPDDONE    0x0800
#define PHY_DEMO_PHYSTAT_DUPLEX     0x2000
#define PHY_DEMO_PHYSTAT_SPEED      0xc000
#define PHY_DEMO_PHYSTAT_GBIT       0x8000
#define PHY_DEMO_PHYSTAT_100        0x4000

/* write bits to a register */
void phy_demo_writebits(struct phy_device *phydev,
                   u8 reg_num, u16 offset, u16 len, u16 data)
{
    u16 reg, mask;

    if ((len + offset) >= 16)
        mask = 0 - (1 << offset);
    else
        mask = (1 << (len + offset)) - (1 << offset);

    reg = phy_read(phydev, MDIO_DEVAD_NONE, reg_num);

    reg &= ~mask;
    reg |= data << offset;

    phy_write(phydev, MDIO_DEVAD_NONE, reg_num, reg);
}

/* Configure PHY */
static int phy_demo_config(struct phy_device *phydev)
{
    u16 reg;

    /* EEE initialization */
    phy_write(phydev, MDIO_DEVAD_NONE, PHY_DEMO_PHY_PAGE, 0x00ff);
    phy_write(phydev, MDIO_DEVAD_NONE, 17, 0x214B);
    phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x2144);
    phy_write(phydev, MDIO_DEVAD_NONE, 17, 0x0C28);
    phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x2146);
    phy_write(phydev, MDIO_DEVAD_NONE, 17, 0xB233);
    phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x214D);
    phy_write(phydev, MDIO_DEVAD_NONE, 17, 0xCC0C);
    phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x2159);
    phy_write(phydev, MDIO_DEVAD_NONE, PHY_DEMO_PHY_PAGE, 0x0000);

    /* SGMII-to-Copper mode initialization */
    if (phydev->interface == PHY_INTERFACE_MODE_SGMII) {
        /* Select page 18 */
        phy_write(phydev, MDIO_DEVAD_NONE, PHY_DEMO_PHY_PAGE, 18);

        /* In reg 20, write MODE[2:0] = 0x1 (SGMII to Copper) */
        phy_demo_writebits(phydev, PHY_DEMO_GENCTR, 0, 3, 
                                       PHY_DEMO_MODE_SGMII);

        /* PHY reset is necessary after changing MODE[2:0] */
        phy_demo_writebits(phydev, PHY_DEMO_GENCTR, 15, 1, 1);

        /* Reset page selection */
        phy_write(phydev, MDIO_DEVAD_NONE, PHY_DEMO_PHY_PAGE, 0);

        udelay(100);
    }

    if (phydev->interface == PHY_INTERFACE_MODE_SGMII) {
	printf("\n\n\nBBBBBB\n\n\n");
        reg = phy_read(phydev, MDIO_DEVAD_NONE,
                        PHY_DEMO_PHY_EXT_SR);

        reg &= ~(PHY_DEMO_HWCFG_MODE_MASK);
        reg |= PHY_DEMO_HWCFG_MODE_SGMII_NO_CLK;
        reg |= PHY_DEMO_HWCFG_FIBER_COPPER_AUTO;

        phy_write(phydev, MDIO_DEVAD_NONE,
                        PHY_DEMO_PHY_EXT_SR, reg);
    }

    /* soft reset */
    phy_reset(phydev);

    genphy_config_aneg(phydev);
    genphy_restart_aneg(phydev);

    return 0;
}

/* Parse special status register for speed and duplex information */
static int phy_demo_parse_status(struct phy_device *phydev)
{
    unsigned int speed;
    unsigned int mii_reg;

    /* Copper Specific Status Register 1 */
    mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, PHY_DEMO_PHY_STATUS);
    if ((mii_reg & PHY_DEMO_PHYSTAT_LINK) &&
                  !(mii_reg & PHY_DEMO_PHYSTAT_SPDDONE)) {
        int i = 0;

        printf("Waiting for PHY realtime link\n");
        while (!(mii_reg & PHY_DEMO_PHYSTAT_SPDDONE)) {
            /* Timeout reached? */
            if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
                puts(" TIMEOUT !\n");
                phydev->link = 0;
                return -ETIMEDOUT;
            }

            if ((i++ % 1000) == 0)
                putc('.');
            udelay(1000);
            mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, 
                                PHY_DEMO_PHY_STATUS);
        }
        puts(" done\n");
        udelay(500000); /* another 500 ms */
    } else {
        if (mii_reg & PHY_DEMO_PHYSTAT_LINK)
            phydev->link = 1;
        else
            phydev->link = 0;
    }
    if (mii_reg & PHY_DEMO_PHYSTAT_DUPLEX)
        phydev->duplex = DUPLEX_FULL;
    else
        phydev->duplex = DUPLEX_HALF;

    speed = mii_reg & PHY_DEMO_PHYSTAT_SPEED;

    switch (speed) {
    case PHY_DEMO_PHYSTAT_GBIT:
        phydev->speed = SPEED_1000;
        break;
    case PHY_DEMO_PHYSTAT_100:
        phydev->speed = SPEED_100;
        break;
    default:
        phydev->speed = SPEED_10;
        break;
    }

    return 0;
}

/* Startup PHY */
static int phy_demo_startup(struct phy_device *phydev)
{
    int ret;

    ret = genphy_update_link(phydev);
    if (ret)
        return ret;

    return phy_demo_parse_status(phydev);
}

static struct phy_driver phy_demo_driver = {
    .name = DEV_NAME,
    .uid  = PHY_DEMO_ID,
    .mask = PHY_DEMO_ID_MASK,
    .features = PHY_GBIT_FEATURES,
    .config   = &phy_demo_config,
    .startup  = &phy_demo_startup,
    .shutdown = &genphy_shutdown,
};

int phy_demo_init(void)
{
    phy_register(&phy_demo_driver);

    return 0;
}
