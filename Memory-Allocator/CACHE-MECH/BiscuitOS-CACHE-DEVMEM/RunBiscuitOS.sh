#!/bin/ash

# Install Module
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-DEVMEM-default.ko
APP &
