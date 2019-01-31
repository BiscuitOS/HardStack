/*
 * DTS: of_mdiobus_register
 *
 * int of_mdiobus_register(struct mii_bus *mdio, struct device_node *np)
 *
 * (C) 2019.01.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Private DTS file: DTS_demo.dtsi
 *
 * / {
 *        DTS_demo {
 *                compatible = "DTS_demo, BiscuitOS";
 *                status = "okay";
 *                phy-handle = <&phy1>;
 *        };
 *
 *        mdio {
 *                compatible = "DTS_mdio, BiscuitOS";
 *                #address-cells = <0x1>;
 *                #size-cells = <0x1>;
 *
 *                phy0: phy@0 {
 *                        reg = <0x0>;
 *                };
 *
 *                phy1: phy@1 {
 *                        reg = <0x1>;
 *                };
 *        };
 * };
 *
 * On Core dtsi:
 *
 * include "DTS_demo.dtsi"
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of_platform.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/of_mdio.h>
#include <linux/mii.h>

/* define name for device and driver */
#define DEV_NAME      "DTS_demo"
#define MII_PHY_ID    0x2
#define PHY_ID        0x1
#define DTS_PHY_ID    0x141

/* Fixup PHY register on page 0 */
static unsigned short fixup_phy_copper_regs[] = {
    0x00,       /* Copper Control */
    0x00,       /* Copper Status */
    0x141,      /* PHY Identifier 1 */
    0x9c0,      /* PHY Identifier 2 */
    0x280,      /* Auto-Negotiation Advertisement Register */
    0x00,       /* Link Partner Ability Register */
    0x00,       /* Auto-Negotiation Expansion Register */
    0x00,       /* Next Page Transmit Register */
    0x00,       /* Link Partner Next Page Register */
    0x00,       /* 1000Base-T Control Register */
    0x00,       /* 1000Base-T Status Register */
    0x00,       
    0x00,
    0x00,
    0x00,
    0x00,       /* Extended Status Register */
    0x00,       /* Copper Specific Control Register 1 */
    0x00,       /* Copper Specific Status Register 1 */
    0x00,       /* Interrupt Enable Register */
    0x00,       /* Copper Specific Status Register 2 */
    0x00,       
    0x00,       /* Receive Error Control Register */
    0x00,       /* Page Address */
    0x00,
    0x00,
    0x00,
    0x00,       /* Copper Specific Control Register 2 */
    0x00,
    0x00,
    0x00,
    0x00,
};

/*
 * fixup mdio bus read
 *
 *  @bus: mdio bus
 *  @phy_addr: PHY device ID which range 0 to 31.
 *  @reg_num: Register address which range 0 to 31 on MDIO Clause 22.
 *
 *  Return special register value.
 */
static int DTS_mdio_read(struct mii_bus *bus, int phy_addr, int reg_num)
{
    struct platform_device *pdev = bus->priv;
    struct device_node *np = pdev->dev.of_node;
    const phandle *ph;
    struct device_node *phy;
    int of_phy_id;
    
    /* find PHY handle on current device_node */
    ph = of_get_property(np, "phy-handle", NULL);

    /* Find child node by handle */
    phy = of_find_node_by_phandle(be32_to_cpup(ph));
    if (!phy) {
        printk("Unable to find device child node\n");
        return -EINVAL;
    }

    /* Read PHY ID on MDIO bus */
    of_property_read_u32(phy, "reg", &of_phy_id);
    if (of_phy_id < 0 || of_phy_id > 31) {
        printk("Invalid phy id from DT\n");
        return -EINVAL;
    }
    
    /* Read Special PHY device by PHY ID */
    if (phy_addr != of_phy_id)
        return -EINVAL;

    return fixup_phy_copper_regs[reg_num];
}

/*
 * fixup mdio bus write
 *
 *  @bus: mdio bus
 *  @phy_addr: PHY device ID which range 0 to 31.
 *  @reg_num: Register address which range 0 to 31 on MDIO Clause 22.
 *  @val: value need to write.
 *
 *  Return 0 always.
 */
static int DTS_mdio_write(struct mii_bus *bus, int phy_addr, int reg_num,
                              u16 val)
{
    /* Forbidding any written */

    return 0;
}

/* probe platform driver */
static int DTS_demo_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    struct mii_bus *bus;
    unsigned short phy_id;
    struct phy_device *phy;

    printk("DTS Probe Entence...\n");

    bus = mdiobus_alloc();
    if (!bus) {
        printk("Unable to allocate memory to mii_bus.\n");
        return -EINVAL;
    }

    bus->name  = "DTS_mdio";
    bus->read  = &DTS_mdio_read;
    bus->write = &DTS_mdio_write;
    snprintf(bus->id, MII_BUS_ID_SIZE, "%s-mii", dev_name(&pdev->dev));
    bus->parent = &pdev->dev;
    bus->priv = pdev;

    platform_set_drvdata(pdev, bus);

    /* Register MDIO bus by DT */
    of_mdiobus_register(bus, np);

    /* MDIO read test */
    phy_id = bus->read(bus, PHY_ID, MII_PHY_ID);
    if (phy_id == DTS_PHY_ID) {
        phy = get_phy_device(bus, PHY_ID, 0);
        if (!phy) {
            printk("Unable to get PHY device.\n");
            return -EINVAL;
        }
        printk("PHY device ID: %#x\n", phy->phy_id);
    }

    return 0;
}

/* remove platform driver */
static int DTS_demo_remove(struct platform_device *pdev)
{
    return 0;
}

static const struct of_device_id DTS_demo_of_match[] = {
    { .compatible = "DTS_demo, BiscuitOS",  },
    { },
};
MODULE_DEVICE_TABLE(of, DTS_demo_of_match);

/* platform driver information */
static struct platform_driver DTS_demo_driver = {
    .probe  = DTS_demo_probe,
    .remove = DTS_demo_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = DEV_NAME, /* Same as device name */
        .of_match_table = DTS_demo_of_match,
    },
};
module_platform_driver(DTS_demo_driver);
