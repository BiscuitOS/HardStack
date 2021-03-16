#!/bin/ash

dd if=/dev/zero of=/BiscuitOS_SWAP bs=1M count=8
sync
chown root:root /BiscuitOS_SWAP

insmod /lib/modules/$(uname -r)/extra/BiscuitOS-SWAP-kernel-default.ko
BiscuitOS-SWAP-userspace-default
