/*
 * Sparse.
 *
 * (C) 2019.07.01 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* sparse macro */
#include <linux/types.h>

/* bitwise: big-endian, little-endian */
typedef unsigned int __bitwise bs_t;

static __init int sparse_demo_init(void)
{
	bs_t a = (__force bs_t)0x12345678;
	bs_t b;

#ifdef __LITTLE_ENDIAN
	printk("little-endian original: %#x\n", a);
#else
	printk("big-endian original:    %#x\n", a);
#endif
	/* Cover to little-endian */
	b = (__force bs_t)cpu_to_le32(a);
	printk("%#x to little-endian: %#x\n", a, b);
	/* Cover to big-endian */
	b = (__force uint32_t)cpu_to_be32(a);
	printk("%#x to bit-endian:    %#x\n", a, b);

	return 0;
}
device_initcall(sparse_demo_init);
