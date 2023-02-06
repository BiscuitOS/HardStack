#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-NOCACHED-Mapping-MMIO-Userspace-default.ko
APP &
cat /sys/kernel/debug/x86/pat_memtype_list
