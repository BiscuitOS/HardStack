#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-PAGING-MMIO-MAPPING-default.ko
cat /sys/kernel/debug/x86/pat_memtype_list
