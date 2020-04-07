#!/bin/ash

# Running kvm
#
# (C) 2020.03.03 BuddyZhang1 <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

OPT=$1

mount_fs()
{
	insmod /lib/modules/$(uname -r)/extra/kvm_module-0.0.1.ko
	insmod /lib/modules/$(uname -r)/extra/kvm-intel-bs.ko
}

umount_fs()
{
	rmmod kvm-intel-bs
	rmmod kvm-bs
}

case $OPT in
	"mount")
		mount_fs
	;;
esac
