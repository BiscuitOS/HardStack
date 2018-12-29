/*
 * PHY driver on uboot
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

/* Configure PHY */
static int phy_demo_config(struct phy_device *phydev)
{
    return 0;
}

/* Startup PHY */
static int phy_demo_startup(struct phy_device *phydev)
{
    int ret;

    ret = genphy_update_link(phydev);
    if (ret)
        return ret;

    return 0;
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
