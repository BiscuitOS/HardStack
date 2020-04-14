/* 
 *
 *   pgcache_scan sysctl inplementation
 *   Written by renyuan on 2020.1.4
 *
 */

#include "include.h"
#include "extern.h"
#include "pgcache_scan.h"


static int scan_top_min = 0;
static int scan_top_max = 0x7fffffff;
static int scan_mode_min  = 0;
static int scan_mode_max  = 2;
static int scan_deleted_min  = 0;
static int scan_deleted_max  = 1;
static int debug_level_min = 0;
static int debug_level_max = 7;

int sysctl_pgcache_scan_top_n        = 0;
int sysctl_pgcache_scan_mode         = 0;
int sysctl_pgcache_scan_debug_level  = 0;
int sysctl_pgcache_scan_file_deleted_but_used = 0;

int scan_caches_sysctl_handler(struct ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos);

static struct ctl_table pgcache_scan_table[] = {
	{
		.procname	= "pgcache_scan_top_n",
		.data		= &sysctl_pgcache_scan_top_n,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= scan_caches_sysctl_handler,
		.extra1         = &scan_top_min,
		.extra2         = &scan_top_max
	},
	{
		.procname	= "pgcache_scan_mode",
		.data		= &sysctl_pgcache_scan_mode,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1         = &scan_mode_min,
		.extra2         = &scan_mode_max
	},
	{
		.procname	= "pgcache_scan_file_deleted_but_used",
		.data		= &sysctl_pgcache_scan_file_deleted_but_used,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1         = &scan_deleted_min,
		.extra2         = &scan_deleted_max
	},
	{
		.procname	= "debug_output_level",
		.data		= &sysctl_pgcache_scan_debug_level,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1         = &debug_level_min,
		.extra2         = &debug_level_max
	},
	{ /* sentinel */ }
};

/* /proc/sys/vm/pgcache_scan */
static struct ctl_table_header *pgcache_scan_sysctl_header = NULL;
static struct ctl_path base_path = {
    .procname = "vm/pgcache_scan"
};

/* Sysctl registration.  */
void pgcache_scan_sysctl_register(void)
{
	pgcache_scan_sysctl_header = register_sysctl_paths(&base_path, pgcache_scan_table);
}

/* Sysctl deregistration.  */
void pgcache_scan_sysctl_unregister(void)
{
	if (pgcache_scan_sysctl_header)
	    unregister_sysctl_table(pgcache_scan_sysctl_header);
}

