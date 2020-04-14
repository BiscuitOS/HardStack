/*
 * written by renyuan on 2019.12.28
 *
 *
 */
#include "include.h"
#include "extern.h"
#include "pgcache_scan.h"

MODULE_AUTHOR("renyuan");
MODULE_DESCRIPTION("scan pagecache, and scan file-deleted which is being openned !! ");
MODULE_LICENSE("GPL");


/* our kernel parameter  */
unsigned long kallsyms_lookup_name_addr = 0UL;
module_param(kallsyms_lookup_name_addr, ulong, 0644);
MODULE_PARM_DESC(kallsyms_lookup_name_addr, "It is for Hooking, Because of no EXPROT_SYMBOL(kallsyms_lookup_name)");

/* Lookup the address for this symbol. Returns 0 if not found. */
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
extern kallsyms_lookup_name_t kallsyms_lookup_name_func;

kallsyms_lookup_name_t kallsyms_lookup_name_func = NULL;



int get_kallsyms_lookup_name_function(void);
int get_kernel_not_export_function_and_data(void);





/* Lookup the address for this symbol. Returns 0 if not found. */
int get_kallsyms_lookup_name_function(void)
{
    kallsyms_lookup_name_func = (kallsyms_lookup_name_t) kallsyms_lookup_name_addr;
    if (((kallsyms_lookup_name_addr & 0xffffffff00000000UL) != 0xffffffff00000000UL) || (kallsyms_lookup_name_func == NULL))
        return -1;
    return 0;
}

int get_kernel_not_export_function_and_data(void)
{
    if ((_super_blocks = (void *)kallsyms_lookup_name_func("super_blocks")) == NULL) {
    	printk("super_blocks       = 0x%p\n", _super_blocks);
        return -1;
    }
    printk("super_blocks           = 0x%p\n", (void *)_super_blocks);

    if ((_sb_lock = (void *)kallsyms_lookup_name_func("sb_lock")) == NULL) {
    	printk("sb_lock            = 0x%p\n", _sb_lock);
        return -1;
    }
    printk("sb_lock                = 0x%p\n", (void *)_sb_lock);

    if ((inode_sb_list_lock = (void *)kallsyms_lookup_name_func("inode_sb_list_lock")) == NULL) {
    	printk("inode_sb_list_lock = 0x%p\n", inode_sb_list_lock);
        return -1;
    }
    printk("inode_sb_list_lock     = 0x%p\n", (void *)inode_sb_list_lock);

    if ((iterate_supers_function = (iterate_supers_addr)kallsyms_lookup_name_func("iterate_supers")) == NULL) {
        printk("iterate_supers     = 0x%p\n", iterate_supers_function);
        return -1;
    }
    printk("iterate_supers         = 0x%p\n", iterate_supers_function);

    return 0;
}

//========================================================================================
/* kernel modules init(entry) function */
static __init int init(void)
{
    if (get_kallsyms_lookup_name_function() != 0)
        return -1;
    printk("kallsyms_lookup_name      = 0x%p\n", (void *)kallsyms_lookup_name_addr);
    printk("kallsyms_lookup_name_func = 0x%p\n", (void *)kallsyms_lookup_name_func);

    if (get_kernel_not_export_function_and_data() != 0) 
        return -1;

//    iterate_supers_function(scan_inodes_pagecache_one_sb, NULL);
//    scan_process_inodes_pagecache();
//    print_top_num(20);
    pgcache_scan_sysctl_register();
    printk("Pgcache scan say: hello !!!\n");
    return 0;
}


/* kernle modules exit function */
static __exit void fini(void)
{
    pgcache_scan_sysctl_unregister();
    order_list_clear();
    printk("Pgcache say: goodbye !!!\n");
    return;
}

module_init(init);
module_exit(fini);
