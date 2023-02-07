#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-UNCACHE-ioremap_uc-with-MMIO-default.ko
cat /sys/kernel/debug/x86/pat_memtype_list
