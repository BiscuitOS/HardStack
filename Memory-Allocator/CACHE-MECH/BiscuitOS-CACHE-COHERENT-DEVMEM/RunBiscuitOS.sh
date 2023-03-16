#!/bin/ash

# Install Module
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-COHERENT-DEVMEM-default.ko
APP &
sleep 0.1
APP1
