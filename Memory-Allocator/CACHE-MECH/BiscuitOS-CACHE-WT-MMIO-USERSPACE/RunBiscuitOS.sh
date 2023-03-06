#!/bin/ash

# Install Module
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-WT-MMIO-USERSPACE-default.ko
APP &
sleep 0.1
cat /sys/kernel/debug/x86/pat_memtype_list
cat /proc/iomem 
