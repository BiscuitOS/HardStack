#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-PAGING-PFNMAP-PFNREMAP-default.ko
APP &
sleep 0.1
cat /sys/kernel/debug/x86/pat_memtype_list
