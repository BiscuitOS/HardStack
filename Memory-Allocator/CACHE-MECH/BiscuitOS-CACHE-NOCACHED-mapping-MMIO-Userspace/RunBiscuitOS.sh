#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-NOCACHE--mapping-MMIO-Userspace-default.ko
APP &
cat /sys/kernel/debug/x86/pat_memtype_list
