#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-OOM-kernel-default.ko 

cat /proc/buddyinfo
