#/bin/ash
[ -f /sys/kernel/debug/x86/cpa_stats ] && cat /sys/kernel/debug/x86/cpa_stats
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-WB-MEM-KERNEL-default.ko
[ -f /sys/kernel/debug/x86/cpa_stats ] && cat /sys/kernel/debug/x86/cpa_stats
