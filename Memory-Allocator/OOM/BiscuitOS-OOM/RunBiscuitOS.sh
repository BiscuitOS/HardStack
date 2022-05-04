#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-OOM-default.ko 

cat /proc/buddyinfo
