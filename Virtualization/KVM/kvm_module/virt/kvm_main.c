/*
 * kvm_main.c
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>

int kvm_init_bs(void *opaque, unsigned vcpu_size, unsigned vcpu_align,
				struct module *module)
{
	printk("SB\n");
}
EXPORT_SYMBOL_GPL(kvm_init_bs);
