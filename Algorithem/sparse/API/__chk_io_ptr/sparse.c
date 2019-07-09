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

static __init int sparse_demo_init(void)
{
	unsigned long __user   *puser;
	unsigned long __kernel *pkernel;
	unsigned long __iomem  *piomem;
	
	/* Check iomem space pointer */
	__chk_io_ptr(puser);
	__chk_io_ptr(pkernel);
	__chk_io_ptr(piomem);

	puser = NULL;
	pkernel = NULL;
	piomem = NULL;

	return 0;
}
device_initcall(sparse_demo_init);
