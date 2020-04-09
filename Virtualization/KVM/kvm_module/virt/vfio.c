/*
 * vfio.c
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/kvm_host.h>
#include "kvm/internal.h"

static int kvm_vfio_create_bs(struct kvm_device *dev, u32 type)
{
	BS_DUP();
	return 0;
}

static void kvm_vfio_destroy_bs(struct kvm_device *dev)
{
	BS_DUP();
}

static int kvm_vfio_set_attr_bs(struct kvm_device *dev,
				struct kvm_device_attr *attr)
{
	BS_DUP();
	return 0;
}

static int kvm_vfio_has_attr_bs(struct kvm_device *dev,
				struct kvm_device_attr *attr)
{
	BS_DUP();
	return 0;
}

static struct kvm_device_ops kvm_vfio_ops_bs = {
	.name     = "kvm-vfio-bs",
	.create   = kvm_vfio_create_bs,
	.destroy  = kvm_vfio_destroy_bs,
	.set_attr = kvm_vfio_set_attr_bs,
	.has_attr = kvm_vfio_has_attr_bs,
};

int kvm_vfio_ops_init_bs(void)
{
	return kvm_register_device_ops_bs(&kvm_vfio_ops_bs, 
						KVM_DEV_TYPE_VFIO);
}
