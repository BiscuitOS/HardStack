/*
 * minix-fs filesytem -- dir
 *
 * (C) 2020.03.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/fs.h>

#include "internal.h"

static int minix_readdir_bs(struct file *file, struct dir_context *ctx)
{
	BS_DUP();
	return 0;
}

const struct file_operations minix_dir_operations_bs = {
	.llseek		= generic_file_llseek,
	.read		= generic_read_dir,
	.iterate_shared	= minix_readdir_bs,
	.fsync		= generic_file_fsync,
};
