#!/bin/ash

# Install Module
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-MODE-COHERENT-RSVDMEM-default.ko
APP &
sleep 0.1
APP1
