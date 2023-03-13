#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-IOREMAP-CHANGE-MEMTYPE-default.ko
cat /sys/kernel/debug/x86/pat_memtype_list
