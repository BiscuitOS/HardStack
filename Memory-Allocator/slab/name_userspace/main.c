/*
 * NAME (kstrdup/kasprintf) Memory Allocator
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "linux/buddy.h"
#include "linux/slub.h"

/* name allocate */
static int instance_name_alloc(void)
{
	char *tmp = "BiscuitOS";
	char *tmx = "BiscuitOS-SpaceX";
	const char *name_memory;
	const char *name_dup;

	name_memory = kstrdup_const(tmp, GFP_KERNEL);
	name_dup = kstrdup(tmx, GFP_KERNEL);

	printk("Kstrdup_const: %s\n", name_memory);
	printk("kstrdup:       %s\n", name_dup);

	kfree(name_dup);
	kfree_const(name_memory);
	return 0;
}

/* format name memory alloc */
static int instance_format_name_alloc(void)
{
	char *tmx = "BiscuitOS";
	int idx = 0x90;
	const char *name_format;
	
	name_format = kasprintf(GFP_KERNEL, "%s-%x", tmx, idx);
	printk("Format name:   %s\n", name_format);
	kfree(name_format);
	return 0;
}

int main()
{
	unsigned long *p;

	memory_init();

	/* Initialize Slub Allocator */
	kmem_cache_init();

	/* Running instance */
	instance_name_alloc();
	instance_format_name_alloc();

	memory_exit();
	return 0;
}
