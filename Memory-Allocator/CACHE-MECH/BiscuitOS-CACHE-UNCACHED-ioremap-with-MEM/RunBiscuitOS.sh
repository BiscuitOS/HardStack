#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-UNCACHED-ioremap-with-MEM-default.ko
cat /sys/kernel/debug/x86/pat_memtype_list
