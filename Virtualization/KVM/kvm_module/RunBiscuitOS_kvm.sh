#!/bin/ash

# Running kvm
#
# (C) 2020.03.03 BuddyZhang1 <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

OPT=$1
ARGS=$2

init_filter()
{
	echo "*_bs*" > /sys/kernel/debug/tracing/set_ftrace_filter
	cat /sys/kernel/debug/tracing/set_ftrace_filter
	echo function > /sys/kernel/debug/tracing/current_tracer
	echo 1 > /sys/kernel/debug/tracing/tracing_on
	echo "Staring tracing .... :)"
}

trace_on()
{
	echo 1 > /sys/kernel/debug/tracing/tracing_on
}

trace_off()
{
	echo 0 > /sys/kernel/debug/tracing/tracing_on
}

show_trace()
{
	cat /sys/kernel/debug/tracing/trace
}

mount_fs()
{
	insmod /lib/modules/$(uname -r)/extra/kvm_module-0.0.1.ko
	insmod /lib/modules/$(uname -r)/extra/kvm-intel-bs.ko
	kvm_userspace-0.0.1
}

umount_fs()
{
	rmmod kvm-intel-bs
	rmmod kvm-bs
}

case $OPT in
	"init")
		init_filter
	;;
	"on")
		trace_on
	;;
	"off")
		trace_off
	;;
	"show")
		show_trace
	;;
	"mount")
		mount_fs
	;;
esac
