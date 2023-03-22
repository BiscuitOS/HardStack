#!/bin/ash

# Install Module
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-MODE-COHERENT-DEVMMIO-default.ko
APP &
sleep 0.1
APP1
cat /sys/kernel/debug/x86/pat_memtype_list
cat /proc/iomem 
