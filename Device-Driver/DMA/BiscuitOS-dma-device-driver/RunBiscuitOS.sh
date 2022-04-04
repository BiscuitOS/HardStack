#!/bin/ash

insmod /lib/modules/$(uname -r)/extra/BiscuitOS-dma-device-driver-default.ko
BiscuitOS-dma-userland-default
