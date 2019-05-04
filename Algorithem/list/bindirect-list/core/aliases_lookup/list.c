/*
 * bindirect-list
 *
 * (C) 20179.04.25 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * bidirect-list
 *
 * +-----------+<--o    +-----------+<--o    +-----------+<--o    +-----------+
 * |           |   |    |           |   |    |           |   |    |           |
 * |      prev |   o----| prev      |   o----| prev      |   o----| prev      |
 * | list_head |        | list_head |        | list_head |        | list_head |
 * |      next |---o    |      next |---o    |      next |---o    |      next |
 * |           |   |    |           |   |    |           |   |    |           |
 * +-----------+   o--->+-----------+   o--->+-----------+   o--->+-----------+
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* header of list */
#include <linux/list.h>

/* struct alias_prop: defined at 'drivers/of/of_private.h' */
#include <linux/device.h>
/**
 * struct alias_prop - Alias property in 'aliases' node 
 * @link:       List node to link the structure in aliases_lookup list
 * @alias:      Alias property name
 * @np:         Pointer to device_node that the alias stands for
 * @id:         Index value from end of alias name
 * @stem:       Alias string without the index
 *
 * The structure represents one alias property of 'aliases' node as
 * an entry in aliases_lookup list.
 */
struct alias_prop {
        struct list_head link;
        const char *alias;
        struct device_node *np;
        int id;
        char stem[0];
};

/* aliases_lookup: define at 'drivers/of/base.c' */
extern struct list_head aliases_lookup;

static __init int bindirect_demo_init(void)
{
	struct alias_prop *ap;

	list_for_each_entry(ap, &aliases_lookup, link)
		printk("%s\n", ap->alias);

	return 0;
}
device_initcall(bindirect_demo_init);
