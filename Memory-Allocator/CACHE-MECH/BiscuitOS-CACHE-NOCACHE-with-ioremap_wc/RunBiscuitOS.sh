#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-NOCACHE-with-ioremap_wc-default.ko
cat /sys/kernel/debug/x86/pat_memtype_list
