#!/bin/ash

dd if=/dev/zero of=/BiscuitOS_swap bs=1M count=8
sync
chown root:root /BiscuitOS_swap

insmod /lib/modules/$(uname -r)/extra/vswap-mod-default.ko
vswap-app-default
