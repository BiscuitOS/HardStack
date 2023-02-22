#!/bin/ash

# Install Module
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-UNCACHE-MMIO-USERSPACE-default.ko
APP &
sleep 0.1
cat /sys/kernel/debug/x86/pat_memtype_list
cat /proc/iomem 
