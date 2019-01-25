/*
 * Init-call order
 *
 * (C) 2019.01.25 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Linux has different initialation order on different segment.
 * Above all, system will follow this order to initialize devices.
 * For:
 *   early_initcall()
 *   pure_initcall()
 *   core_initcall()
 *   core_initcall_sync()
 *   postcore_initcall()
 *   postcore_initcall_sync()
 *   arch_initcall()
 *   arch_initcall_sync()
 *   subsys_initcall()
 *   subsys_initcall_sync()
 *   fs_initcall()
 *   fs_initcall_sync()
 *   rootfs_initcall()
 *   device_initcall()
 *   device_initcall_sync()
 *   late_initcall()
 *   late_initcall_sync()
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* Early init */
int __init __early_initcall(void)
{
    printk("Early_initcall on first.\n");
    return 0;
}
early_initcall(__early_initcall);

/* Level0 init */
int __init __pure_initcall(void)
{
    printk("Pure initcall on level 0.\n");
    return 0;
}
pure_initcall(__pure_initcall);

/* Level1 init */
int __init __core_initcall(void)
{
    printk("Core initcall on level 1.\n");
    return 0;
}
core_initcall(__core_initcall);

/* Level1s init */
int __init __core_initcall_sync(void)
{
    printk("Core initcall sync on level 1s.\n");
    return 0;
}
core_initcall_sync(__core_initcall_sync);

/* Level2 init */
int __init __postcore_initcall(void)
{
    printk("Postcore_initcall on level 2.\n");
    return 0;
}
postcore_initcall(__postcore_initcall);

/* Level2s init */
int __init __postcore_initcall_sync(void)
{
    printk("Postcore_initcall_sync on level 2s.\n");
    return 0;
}
postcore_initcall_sync(__postcore_initcall_sync);

/* Level3 init */
int __init __arch_initcall(void)
{
    printk("Arch_initcall on level 3.\n");
    return 0;
}
arch_initcall(__arch_initcall);

/* Level3s init */
int __init __arch_initcall_sync(void)
{
    printk("Arch_initcall_sync on level 3s.\n");
    return 0;
}
arch_initcall_sync(__arch_initcall_sync);

/* Level4 init */
int __init __subsys_initcall(void)
{
    printk("Subsys_initcall on level 4.\n");
    return 0;
}
subsys_initcall(__subsys_initcall);

/* Level4s init */
int __init __subsys_initcall_sync(void)
{
    printk("Subsys_initcall_sync on level 4s.\n");
    return 0;
}
subsys_initcall_sync(__subsys_initcall_sync);

/* Level5 init */
int __init __fs_initcall(void)
{
    printk("Fs_initcall on level 5.\n");
    return 0;
}
fs_initcall(__fs_initcall);

/* Level5s init */
int __init __fs_initcall_sync(void)
{
    printk("Fs_initcall_sync on level 5s.\n");
    return 0;
}
fs_initcall_sync(__fs_initcall_sync);

/* Level5.x init */
int __init __rootfs_initcall(void)
{
    printk("Rootfs_initcall on level 5.x.\n");
    return 0;
}
rootfs_initcall(__rootfs_initcall);

/* Level6 init */
int __init __device_initcall(void)
{
    printk("Device_initcall on level 6.\n");
    return 0;
}
device_initcall(__device_initcall);

/* Level6s init */
int __init __device_initcall_sync(void)
{
    printk("Device_initcall_sync on level 6s.\n");
    return 0;
}
device_initcall_sync(__device_initcall_sync);

/* Level7 init */
int __init __late_initcall(void)
{
    printk("Late_initcall on level 7.\n");
    return 0;
}
late_initcall(__late_initcall);

/* Level7s init */
int __init __late_initcall_sync(void)
{
    printk("Late_initcall_sync on level 7s.\n");
    return 0;
}
late_initcall_sync(__late_initcall_sync);
