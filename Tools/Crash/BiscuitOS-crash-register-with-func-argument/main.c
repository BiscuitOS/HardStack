/*
 * Register with Function argument
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

int data[] = { 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 
	       11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };

int BiscuitOS_func_arguments(int  *a0, int  *a1, int  *a2, int  *a3,
			     int  *a4, int  *a5, int  *a6, int  *a7, 
			     int  *a8, int  *a9, int *a10, int *a11,
			     int *a12, int *a13, int *a14, int *a15,
			     int *a16, int *a17, int *a18, int *a19)
{
	int *p = NULL;

	*p = 88520;

	printk("%d %d %d %d %d %d %d %d %d %d\n"
	       "%d %d %d %d %d %d %d %d %d %d", 
			*a0,  *a1,  *a2,  *a3,  *a4,
			*a5,  *a6,  *a7,  *a8,  *a9,
			*a10, *a11, *a12, *a13, *a14,
			*a15, *a16, *a17, *a18, *a19);

	return *p;
}

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	printk("Address for data: %#lx\n", (unsigned long)data);

	BiscuitOS_func_arguments(&data[0],  &data[1],  &data[2],  &data[3],
				 &data[4],  &data[5],  &data[6],  &data[7],
				 &data[8],  &data[9],  &data[10], &data[11],
				 &data[12], &data[13], &data[14], &data[15],
				 &data[16], &data[17], &data[18], &data[19]);

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("CRASH on BiscuitOS");
