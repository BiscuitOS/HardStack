/*
 * MDIO/SMI/MIIM on Kernel
 *
 * (C) 2018.12.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/phy_fixed.h>

#define MII_BUS         "mii_demo"

static struct mii_bus *mii_demo;

extern int swphy_read_reg(int reg, const struct fixed_phy_status *state);

/* mdio read entence */
static int mii_demo_read(struct mii_bus *bus, int phys_addr, int reg)
{
    struct fixed_phy_status state;

    /* setup real MDIO control register and transfer data */

    return swphy_read_reg(reg, &state);
}

/* mdio write entence */
static int mii_demo_write(struct mii_bus *bus, int phy_addr, int reg, 
                                 unsigned short val)
{
    return 0;
}

static int __init mdio_demo_init(void)
{
    int ret;

    /* Allocate MII bus */
    mii_demo = mdiobus_alloc();
    if (mii_demo == NULL) {
        ret = -ENOMEM;
        goto err;
    }

    /* setup mii bus */
    snprintf(mii_demo->id, MII_BUS_ID_SIZE, "mii_demo-0");
    mii_demo->name  = MII_BUS;
    mii_demo->read  = &mii_demo_read;
    mii_demo->write = &mii_demo_write;

    /* Register mdio bus */
    ret = mdiobus_register(mii_demo);
    if (ret)
        goto err_alloc;   

    return 0;

err_alloc:
    mdiobus_free(mii_demo);

err:
    return ret;
}

static void __exit mdio_demo_exit(void)
{
    mdiobus_unregister(mii_demo);

    mdiobus_free(mii_demo);    
}

module_init(mdio_demo_init);
module_exit(mdio_demo_exit);
MODULE_LICENSE("GPL v2");
