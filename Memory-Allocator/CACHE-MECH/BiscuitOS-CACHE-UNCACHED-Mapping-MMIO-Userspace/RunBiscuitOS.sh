#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-UNCACHED-Mapping-MMIO-Userspace-default.ko
APP &
sleep 0.1
cat /sys/kernel/debug/x86/pat_memtype_list
