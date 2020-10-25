/*
 * Kernel write to Userspace file on BiscuitOS
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

#include <linux/fs.h>
#include <asm/uaccess.h>

/* The path for Userspace file */
#define BISCUITOS_FILE		"/tmp/BiscuitOS.info"
/* Buffer length */
#define MAX_BUFFER_SIZE		256

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	char buffer[MAX_BUFFER_SIZE];
	struct file *filp = NULL;
	mm_segment_t fs;
	loff_t pos = 0;
	
	/* Open Userspace file */
	filp = filp_open(BISCUITOS_FILE, O_RDWR | O_CREAT, 0644);
	if (IS_ERR(filp)) {
		printk("ERROR[%ld]: Open %s failed.\n", PTR_ERR(filp),
							BISCUITOS_FILE);
		return PTR_ERR(filp);
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	/* Write data into Userspace file */
	sprintf(buffer, "BiscuitOS");
	buffer[strlen(buffer)] = '\n';
	pos = 0;
	kernel_write(filp, buffer, strlen(buffer) + 1, &pos);

	/* Close Userspace file */
	filp_close(filp, NULL);
	set_fs(fs);

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
MODULE_DESCRIPTION("Write Userspace file on BiscuitOS");
