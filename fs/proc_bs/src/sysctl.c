/*
 * General linux system control interface
 *
 * (C) 2020.03.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysctl.h>

#include "internal.h"

unsigned int sysctl_sched_child_runs_first_bs __read_mostly;

int proc_dointvec_bs(struct ctl_table *table, int write,
			void __user *buffer, size_t *lenp, loff_t *ppos)
{
	BS_DUP();
	return 0;
}

/* The default sysctl tables: */

static struct ctl_table kern_table_bs[] = {
	{
		.procname	= "sched_child_runs_first_bs",
		.data		= &sysctl_sched_child_runs_first_bs,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_bs,
	},
	{ }
};

static struct ctl_table sysctl_base_table_bs[] = {
	{
		.procname	= "kernel",
		.mode		= 0555,
		.child		= kern_table_bs,
	},
	{ }
};

int __init sysctl_init_bs(void)
{
	struct ctl_table_header *hdr;

	hdr = register_sysctl_table_bs(sysctl_base_table_bs);
	return 0;
}
