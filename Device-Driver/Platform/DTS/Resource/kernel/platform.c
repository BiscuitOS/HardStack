/*
 * Platform Resource from DTS
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Private DTS file: DTS_demo.dtsi
 *
 * / {
 *        Platform_demo {
 *                compatible = "Platform_demo, BiscuitOS";
 *                status = "okay";
 *                bool-property;
 *                u8-property = <0x11223344>;
 *                u8-array-property = <0x11223344 0x55667788>;
 *                u16-property = <0xaabbccdd>;
 *                u16-array-property = <0x45678923 0x32563487>;
 *                u32-property = <0x12345678>;
 *                u32-array-property = <0x124adcef 0x20aabbcc>;
 *                u64-property = <0x11223344 0x55667788>;
 *                u64-array-property = <0x21475839 0x7836475d
 *                                      0x874dabef 0x4fdeabbd>;
 *                
 *                gpio-property = <&gpio2 18 1>;
 *                gpio2: gpio_demo@2 {
 *                        compatible = "gpio-controller, BiscuitOS";
 *                        reg = <2>;
 *                        gpio-controller;
 *                        #gpio-cells = <2>;
 *                        ngpios = <32>;
 *                        interrupt-controller;
 *                        #interrupt-celss = <2>;
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
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>

/* DDL Platform Name */
#define DEV_NAME "Platform_demo"

/* Probe: (DDL) Initialize Device */
static int Platform_demo_probe(struct platform_device *pdev)
{
	struct device_node *np = dev_of_node(&pdev->dev);
	struct device_node *gpio_np;
	bool bool_property;
	const phandle *ph;
	u8 u8_property;
	u8 u8_array[8];
	u16 u16_property;
	u16 u16_array[4];
	u32 u32_property;
	u32 u32_array[2];
	u64 u64_property;
	u64 u64_array[2];
	const char *string_property;
	const char *string_array[3];
	int gpio;
	int idx, count;

	/* bool property
	 *   Read 'bool' property from DTS. (Note! bool property
	 *   used to indicate whether exist for property, and
	 *   the bool property is 'ture' when property exists,
	 *   otherwise, bool property is 'false')
	 * Usage:
	 *   1) ranges
	 *   2) interrupts-controll
	 */
	bool_property = of_property_read_bool(np, "bool-property");
	if (bool_property)
		printk("'bool-property' property is ture!\n");
	else
		printk("'bool-property' property is false!\n");
	bool_property = of_property_read_bool(np, "bool-nopresent");
	if (bool_property)
		printk("'bool-nopresent' property is true!\n");
	else
		printk("'bool-nopresent' property is false!\n");
	
	/* U8/Uchar/char single property
	 *   Read one u8 property from DTS. (Note!
	 *   The data endian is big endian on u8 array, so
	 *   you should translate big endian to correct big/little
	 *   endian on your platform after reading data from DTS).
	 *   Such as ARMv7 is little endian, 'of_property_read_u8()'
	 *   will automatically translate data to correct data endian.
	 *
	 *   u8-property = <0x11223344>;
	 */
	of_property_read_u8(np, "u8-property", &u8_property);
	printk("'u8-property' property is %#hhx", u8_property);

	/* U8/Uchar/char array property
	 *   Read a serial of u8 properties from DTS. (Note!
	 *   The data endian is big endian on DTB, so you notice
	 *   whether need to translate big endian to correct big/little
	 *   endian on your platform after reading data from DTS).
	 *   Such as ARMv7 is little endian, 'of_property_read_u8_array()'
	 *   will automatically translate data to correct data endian.
	 *
	 *   u8-array-property = <0x11223344 0x55667788>;
	 */
	of_property_read_u8_array(np, "u8-array-property", u8_array, 8);
	for (idx = 0; idx < 8; idx++)
		printk("U8_Array[%d] %#hhx\n", idx, u8_array[idx]);

	/* U16/Ushort/short single property
	 *   Read one u16 property from DTS. (Note! The data endian
	 *   is big endian on DTB, so you notice whether translate
	 *   big endian to correct big/little endian on your platform
	 *   after reading data from DTS.)
	 *   Such as ARMv7 is little endian, 'of_property_read_u16()' 
	 *   will automatically translate data to correct data endian.
	 *
	 *   u16-property = <0xaabbccdd>;
	 */
	of_property_read_u16(np, "u16-property", &u16_property);
	printk("'u16_property' property is %#hx", u16_property);

	/* U16/Ushort/short array property
	 *   Read a serial of u16 propreties from DTS. (Note! The data
	 *   endian type is big endian on DTB, so you notice whether
	 *   translate big endian to correct big/little endian when you
	 *   invoking related function)
	 *   Such as ARMv7 is little endian, 'of_property_read_u16_array()'
	 *   will automatically translate data into correct data endian
	 *   type.
	 *
	 *   u16-array-property = <0x45678923 0x32563487>;
	 */
	of_property_read_u16_array(np, "u16-array-property", u16_array, 4);
	for (idx = 0; idx < 4; idx++)
		printk("U16_Array[%d] %#hx\n", idx, u16_array[idx]);

	/* U32/Uint/int/unsigned long(32-bit) single property
	 *   Read one u32 property from DTS. (Note! The data endian
	 *   is big endian on DTB, so you notice whether translate
	 *   big endian to correct big/little endian on your platform
	 *   after reading data from DTS.)
	 *   Such as ARMv7 is little endian, 'of_property_read_u32()' 
	 *   will automatically translate data to correct data endian.
	 *
	 *   u32-property = <0x12345678>;
	 */
	of_property_read_u32(np, "u32-property", &u32_property);
	printk("'u32_property' property is %#x", u32_property);

	/* U32/Uint/int/unsigned long(32-bit) array property
	 *   Read a serial of u32 propreties from DTS. (Note! The data
	 *   endian type is big endian on DTB, so you notice whether
	 *   translate big endian to correct big/little endian when you
	 *   invoking related function)
	 *   Such as ARMv7 is little endian, 'of_property_read_u32_array()'
	 *   will automatically translate data into correct data endian
	 *   type.
	 *
	 *   u32-array-property = <0x124adcef 0x20aabbcc>;
	 */
	of_property_read_u32_array(np, "u32-array-property", u32_array, 2);
	for (idx = 0; idx < 2; idx++)
		printk("U32_Array[%d] %#x\n", idx, u32_array[idx]);

	/* U64/Ulong long/long long/unsigned long(64-bit) single property
	 *   Read one u64 property from DTS. (Note! The data endian
	 *   is big endian on DTB, so you notice whether translate
	 *   big endian to correct big/little endian on your platform
	 *   after reading data from DTS.)
	 *   Such as ARMv7 is little endian, 'of_property_read_u64()' 
	 *   will automatically translate data to correct data endian.
	 *
	 *   u64-property = <0x11223344 0x55667788>;
	 */
	of_property_read_u64(np, "u64-property", &u64_property);
	printk("'u64_property' property is %#llx", u64_property);

	/* U64/Ulong long/long long/unsigned long(64-bit) array property
	 *   Read a serial of u64 propreties from DTS. (Note! The data
	 *   endian type is big endian on DTB, so you notice whether
	 *   translate big endian to correct big/little endian when you
	 *   invoking related function)
	 *   Such as ARMv7 is little endian, 'of_property_read_u64_array()'
	 *   will automatically translate data into correct data endian
	 *   type.
	 *
	 *   u64-array-property = <0x21475839 0x7836475d
	 *                         0x874dabef 0x4fdeabbd>;
	 */
	of_property_read_u64_array(np, "u64-array-property", u64_array, 2);
	for (idx = 0; idx < 2; idx++)
		printk("U64_Array[%d] %#llx\n", idx, u64_array[idx]);

	/* String single property
	 *   Read one String property from DTS.
	 *
	 *   string-property = "BiscuitOS";
	 */
	of_property_read_string(np, "string-property", &string_property);
	printk("'string property' proeprty value: %s\n", string_property);

	/*
	 * String array property
	 *   Read a serial of string properties from DTS.
	 *
	 *   string-array-property = "Uboot", "Kernel", "Rootfs";
	 */
	count = of_property_count_strings(np, "string-array-property");
	of_property_read_string_array(np, "string-array-property", 
					string_array, count);
	for (idx = 0; idx < count; idx++)
		printk("String_Array[%d] %s\n", idx, string_array[idx]);
	of_property_read_string_index(np, "string-array-property", 1,
							&string_property);
	printk("String_Array[1] %s\n", string_property); 

	/* Phandle
	 *   Get device node by phandle on DTS.
	 *
	 *   gpio-property = <&gpio2 18 1>;
	 *
	 *   gpio2: gpio_demo@2 {
	 *             compatible = "gpio-controller, BiscuitOS";
	 *             ....
	 *   };
	 */
	ph = of_get_property(np, "gpio-property", NULL);
	if (!ph) {
		printk("Unable to find 'gpio-property' on DTS\n");
		return -EINVAL;
	}
	gpio_np = of_find_node_by_phandle(be32_to_cpup(ph));
	if (!gpio_np) {
		printk("Unable to find 'gpio2'\n");
		return -EINVAL;
	}
	printk("phandle: %s\n", gpio_np->name);

	/* GPIO Controller */
	if (of_get_property(gpio_np, "gpio-controller", NULL)) {
		printk("The number of GPIO on [%s] GPIO-controller\n", 
					gpio_np->name);
	}

	/* GPIO
	 *   Get GPIO number from DTS.
	 *
	 *   gpio-property = <&gpio2 18 1>;
	 */
	gpio = of_get_named_gpio(np, "gpio-property", 0);
	printk("GPIO (of_get_named_gpio): %d\n", gpio);
	gpio = of_get_gpio(np, 18);
	printk("GPIO (of_get_gpio): %d\n", gpio);

	return 0;
}

/* Remove: (DDL) Remove Device (Module) */
static int Platform_demo_remove(struct platform_device *pdev)
{
	/* Device Remove Procedure */
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);
	
	return 0;
}

/* Shutdown: (DDL) Power-off/Shutdown */
static void Platform_demo_shutdown(struct platform_device *pdev)
{
	/* Device Shutdown/Power-off Procedure */
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);
}

/* Suspend: (DDL) Suspend (schedule) Sleep */
static int Platform_demo_suspend(struct platform_device *pdev, 
							pm_message_t state)
{
	/* Device Suspend/Sleep Procedure */
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);

	return 0;
}

/* Resume: (DDL) (schedule) From Suspend/Sleep */
static int Platform_demo_resume(struct platform_device *pdev)
{
	/* Device Resume Procedure */
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);

	return 0;
}

static const struct of_device_id Platform_demo_of_match[] = {
	{ .compatible = "Platform_demo, BiscuitOS", },
	{ },
};
MODULE_DEVICE_TABLE(of, Platform_demo_of_match);

/* Platform Driver Information */
static struct platform_driver Platform_demo_driver = {
	.probe    = Platform_demo_probe,
	.remove   = Platform_demo_remove,
	.shutdown = Platform_demo_shutdown,
	.suspend  = Platform_demo_suspend,
	.resume   = Platform_demo_resume,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
		.of_match_table	= Platform_demo_of_match,
	},
};
module_platform_driver(Platform_demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Platform Resource from DTS");
