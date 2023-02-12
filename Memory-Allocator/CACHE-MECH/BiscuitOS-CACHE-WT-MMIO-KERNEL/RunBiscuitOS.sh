#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-WT-MMIO-KERNEL-default.ko
cat /sys/kernel/debug/x86/pat_memtype_list
