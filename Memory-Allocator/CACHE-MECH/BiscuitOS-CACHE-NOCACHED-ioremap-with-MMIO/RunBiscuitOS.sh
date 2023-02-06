#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-NOCACHE-with-ioremap-default.ko
cat /sys/kernel/debug/x86/pat_memtype_list
