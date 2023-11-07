#/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-PAGING-PFNMAP-MEMREMAP-default.ko
sleep 0.1
cat /sys/kernel/debug/x86/pat_memtype_list
