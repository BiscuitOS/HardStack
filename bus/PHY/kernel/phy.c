#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/mii.h>
#include <linux/phy.h>

#define PHY_DEMO_ID         0x01410dd1
#define PHY_DEMO_ID_MASK    0xfffffff0
#define PHY_DEMO_NAME       "PHY demo 0"

#define PHY_DEMO_PHY_PAGE                0x16
#define PHY_DEMO_GEN_CTRL1               0x14
#define PHY_DEMO_GEN_CTRL1_RESET         0x8000 /* soft reset */
#define PHY_DEMO_GEN_CTRL1_MODE_MASK     0x7    
#define PHY_DEMO_GEN_CTRL1_MODE_SGMII    0x1    /* SGMII to copper */

/*
 * Initialize PHY mode and LED...
 */
static int phy_demo_config_init(struct phy_device *phydev)
{
    int err;
    int temp;

    /* SGMII-to-Copper mode initialization */
    if (phydev->interface == PHY_INTERFACE_MODE_SGMII) {
        /* Select page 18 */
        err = phy_write(phydev, PHY_DEMO_PHY_PAGE, 18);
        if (err < 0)
            return err;

        /* In reg 20, write MODE[2:0] = 0x1 (SGMII to Copper) */
        temp = phy_read(phydev, PHY_DEMO_GEN_CTRL1);
        temp &= ~PHY_DEMO_GEN_CTRL1_MODE_MASK;
        temp |= PHY_DEMO_GEN_CTRL1_MODE_SGMII;
        err = phy_write(phydev, PHY_DEMO_GEN_CTRL1, temp);
        if (err < 0)
            return err;

        /* PHY reset is necessary after changing MODE[2:0] */
        temp |= PHY_DEMO_GEN_CTRL1_RESET;
        err = phy_write(phydev, PHY_DEMO_GEN_CTRL1, temp);
        if (err < 0)
            return err;
    }

    return 0;
}

/*
 * PHY Auto-Negotitation
 */
static int phy_demo_config_aneg(struct phy_device *phydev)
{
    /* PHY HW Auto-Negotitation */
    return 0;
}

/*
 * Read PHY status.
 */
static int phy_demo_read_status(struct phy_device *phydev)
{
    /* PHY HW link status */
    return 0;
}

/* Read PHY interrupt status */
static int phy_demo_ack_interrupt(struct phy_device *phydev)
{
    return 0;
}

/* Configure interrupt */
static int phy_demo_config_intr(struct phy_device *phydev)
{
    return 0;
}

static int phy_demo_probe(struct phy_device *phydev)
{
    printk("\n\n\n\nsdafsdfasdfsdfsdDDDDDD\n\n\n\n");
 
    /* HW initialization */
    return 0; 
}

static struct phy_driver phy_demo_drivers[] = {
    {
        .phy_id = PHY_DEMO_ID,
        .phy_id_mask = PHY_DEMO_ID_MASK,
        .name = PHY_DEMO_NAME,
        .features = PHY_GBIT_FEATURES | SUPPORTED_FIBRE,
        .flags = PHY_HAS_INTERRUPT,
        .probe = &phy_demo_probe,
        .config_init = &phy_demo_config_init,
        .config_aneg = &phy_demo_config_aneg,
        .read_status = &phy_demo_read_status,
        .ack_interrupt = &phy_demo_ack_interrupt,
        .config_intr = &phy_demo_config_intr,
    },
};

module_phy_driver(phy_demo_drivers);

static struct mdio_device_id __maybe_unused phy_demo_table[] = {
    { PHY_DEMO_ID,  PHY_DEMO_ID_MASK },
    { }
};

MODULE_DEVICE_TABLE(mdio, phy_demo_table);

MODULE_DESCRIPTION("PHY demo driver");
MODULE_AUTHOR("BuddyZhang1");
MODULE_LICENSE("GPL");
