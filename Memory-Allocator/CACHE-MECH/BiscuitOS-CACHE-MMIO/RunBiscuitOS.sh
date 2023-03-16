#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-MMIO-default.ko
APP &
sleep 0.1
cat /sys/kernel/debug/x86/pat_memtype_list
