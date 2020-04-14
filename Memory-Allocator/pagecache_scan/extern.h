#ifndef __EXTERN_H
#define __EXTERN_H

#include "include.h"

extern unsigned long kallsyms_lookup_name_addr;
extern int get_kallsyms_lookup_name_function(void);

extern int scan_process_inodes_pagecache(void);
extern void scan_inodes_pagecache_one_sb(struct super_block *sb, void *arg);
extern void order_list_clear(void);
extern void print_top_num(int num);

extern int sysctl_pgcache_scan_top_n;
extern int sysctl_pgcache_scan_mode;
extern int sysctl_pgcache_scan_debug_level;
extern int sysctl_pgcache_scan_file_deleted_but_used;

extern void pgcache_scan_sysctl_register(void);
extern void pgcache_scan_sysctl_unregister(void);
#endif
