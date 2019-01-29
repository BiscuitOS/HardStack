/*
 * Platform Bus
 *
 * (C) 2019.01.28 BuddyZhang1 <buddy.zhang@aliyun.com>
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
 *        };
 * };
 *
 * On Core dtsi:
 *
 * include "DTS_demo.dtsi"
 */

/*
 * Klist:
 *      Platform Bus <--> Platform Device <--> Platform Driver
 *      1) All Platform devices hold on 'klist_device' list on Platform bus.
 *      2) All Platform drivers hold on 'klist_driver' list on Platform bus.
 *
 *                        +----------+
 *                        |          |
 *                        | bus_type |
 *                        |       p -|-----o
 *                        |          |     |
 *                        +----------+     |
 *                                         V
 *                        +----------------+
 *                        |                |
 *                        | subsys_private |
 *                     o->|- klist_devices |
 *                     |  |  klist_drivers-|<-o
 *                     |  |                |  |
 *                     |  +----------------+  |
 *                     |                      |
 *                     |                      |
 *                     |                      |
 * +--------+          |                      |    +---------------+
 * |        |          |                      |    |               |
 * | device |          |                      |    | device_driver |
 * |     p -|-------o  |                      |    |            p -|---o
 * |        |       |  |                      |    |               |   |
 * +--------+       |  |                      |    +---------------+   |
 *                  V  |                      |                        |
 * +----------------+  |                      |    +----------------+<-o
 * |                |  |                      |    |                |
 * | device_private |  |                      |    | driver_private |
 * |      knode_bus |<-o                      o<-->|      knode_bus |
 * |                |  |                      |    |                |
 * +----------------+  |                      |    +----------------+
 *                     |                      |
 *                     |                      |
 *                     |                      |
 * +--------+          |                      |    +---------------+
 * |        |          |                      |    |               |
 * | device |          |                      |    | device_driver |
 * |     p -|-------o  |                      |    |            p -|---o
 * |        |       |  |                      |    |               |   |
 * +--------+       |  |                      |    +---------------+   |
 *                  V  |                      |                        |
 * +----------------+  |                      |    +----------------+<-o
 * |                |  |                      |    |                |
 * | device_private |  |                      |    | driver_private |
 * |      knode_bus |<-o                      o<-->|      knode_bus |
 * |                |  |                      |    |                |
 * +----------------+  |                      |    +----------------+
 *                     |                      |
 *                     |                      |
 *                     |                      |
 * +--------+          |                      |    +---------------+
 * |        |          |                      |    |               |
 * | device |          |                      |    | device_driver |
 * |     p -|-------o  |                      |    |            p -|---o
 * |        |       |  |                      |    |               |   |
 * +--------+       |  |                      |    +---------------+   |
 *                  V  |                      |                        |
 * +----------------+  |                      |    +----------------+<-o
 * |                |  |                      |    |                |
 * | device_private |  |                      |    | driver_private |
 * |      knode_bus |<-o                      o<-->|      knode_bus |
 * |                |  |                      |    |                |
 * +----------------+  |                      |    +----------------+
 *                     |                      |
 *                     |                      |
 *                     |                      |
 * +--------+          |                      |    +---------------+
 * |        |          |                      |    |               |
 * | device |          |                      |    | device_driver |
 * |     p -|-------o  |                      |    |            p -|---o
 * |        |       |  |                      |    |               |   |
 * +--------+       |  |                      |    +---------------+   |
 *                  V  |                      |                        |
 * +----------------+  |                      |    +----------------+<-o
 * |                |  |                      |    |                |
 * | device_private |  |                      |    | driver_private |
 * |      knode_bus |<-o                      o<-->|      knode_bus |
 * |                |                              |                |
 * +----------------+                              +----------------+
 *                                           
 */

/*
 * Kset and Kobject: 
 *    Platform bus <--> Platform driver
 *    1) Path on '/sys/bus/platform/'
 *
 *
 *                   +------------------+
 *                   |                  |
 * +----------+      | subsys_private   |
 * |          |      | driver_kset.list |<-----o
 * | bus_type |      |                  |      |
 * |       p -|----->+------------------+      |
 * |          |                                |
 * +----------+                                |
 *                                             |
 *                                             |
 *                       +----------------+    |
 *                       |                |    |
 * +---------------+     | driver_private |    |
 * |               |     |     kobj.entry |<-->o
 * | device_driver |     |                |    |
 * |            p -|---->+----------------+    |
 * |               |                           |
 * +---------------+                           |
 *                                             |
 *                                             |
 *                       +----------------+    |
 *                       |                |    |
 * +---------------+     | driver_private |    |
 * |               |     |     kobj.entry |<-->o
 * | device_driver |     |                |    |
 * |            p -|---->+----------------+    |
 * |               |                           |
 * +---------------+                           |
 *                                             |
 *                                             |
 *                       +----------------+    |
 *                       |                |    |
 * +---------------+     | driver_private |    |
 * |               |     |     kobj.entry |<-->o
 * | device_driver |     |                |    |
 * |            p -|---->+----------------+    |
 * |               |                           |
 * +---------------+                           |
 *                                             |
 *                                             |
 *                       +----------------+    |
 *                       |                |    |
 * +---------------+     | driver_private |    |
 * |               |     |     kobj.entry |<-->o
 * | device_driver |     |                |    
 * |            p -|---->+----------------+    
 * |               |                           
 * +---------------+                           
 *                                             
 */

/*
 * Ref: Platform Driver <---> Platform_driver
 *      1) One driver points to Multiple devices.
 *      2) One device only points to one driver.
 * 
 *
 *
 *                    +---------------+
 *                    |               |
 *                    | device_driver |
 *                    |            p -|---------o
 *                    |               |         |
 *           o------->+---------------+         |
 *           |                                  |
 *           |        +----------------+<-------o
 *           |        |                |
 *           |        | driver_private |
 *           |        |  klist_devices |--------o
 *           |        |                |        |
 *           |        +----------------+        |
 *           |                                  |
 *           |                                  |
 *           |                                  |
 *           |        +--------+                |
 *           |        |        |                |
 *           |        | device |                |
 *           |        |     p -|------------o   |
 *           o<-------| druver |            |   |
 *           |        |        |            |   |
 *           |        +--------+            |   |
 *           |                              |   |
 *           |        +----------------+<---o   |
 *           |        |                |        |
 *           |        | device_private |        |
 *           |        |  knode_driver  |<------>o
 *           |        |                |        |
 *           |        +----------------+        |
 *           |                                  |
 *           |                                  |
 *           |                                  |
 *           |                                  |
 *           |        +--------+                |
 *           |        |        |                |
 *           |        | device |                |
 *           |        |     p -|------------o   |
 *           o<-------| druver |            |   |
 *           |        |        |            |   |
 *           |        +--------+            |   |
 *           |                              |   |
 *           |        +----------------+<---o   |
 *           |        |                |        |
 *           |        | device_private |        |
 *           |        |  knode_driver  |<------>o
 *           |        |                |        |
 *           |        +----------------+        |
 *           |                                  |
 *           |                                  |
 *           |                                  |
 *           |                                  |
 *           |        +--------+                |
 *           |        |        |                |
 *           |        | device |                |
 *           |        |     p -|------------o   |
 *           o--------| druver |            |   |
 *                    |        |            |   |
 *                    +--------+            |   |
 *                                          |   |
 *                    +----------------+<---o   |
 *                    |                |        |
 *                    | device_private |        |
 *                    |  knode_driver  |<------>o
 *                    |                |        
 *                    +----------------+        
 *                                             
 *                                             
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>

/* define name for device and driver */
#define DEV_NAME "DTS_demo"

/* probe platform driver */
static int DTS_demo_probe(struct platform_device *pdev)
{
    printk("DTS demo probe entence.\n");

    return 0;
}

/* remove platform driver */
static int DTS_demo_remove(struct platform_device *pdev)
{
    return 0;
}

/* platform driver information */
static struct platform_driver DTS_demo_driver = {
    .probe  = DTS_demo_probe,
    .remove = DTS_demo_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = DEV_NAME, /* Same as device name */
    },
};
module_platform_driver(DTS_demo_driver);

static struct platform_device Demo_device = {
    .name = DEV_NAME,
    .id   = 0,
};

/* Register a device on early stage. */
static int Demo_device_init(void)
{
    return platform_device_register(&Demo_device);
}
arch_initcall(Demo_device_init);
