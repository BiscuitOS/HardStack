#!/bin/sh

dd if=/dev/zero of=/BiscuitOS_swap bs=1M count=8
sync
chown root:root /BiscuitOS_swap
chmod 600 /BiscuitOS_swap

BiscuitOS-mkswap-swapon-default
