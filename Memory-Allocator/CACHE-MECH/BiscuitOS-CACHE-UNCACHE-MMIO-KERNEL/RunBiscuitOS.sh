#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-UNCACHE-MMIO-KERNEL-default.ko
cat /sys/kernel/debug/x86/pat_memtype_list
