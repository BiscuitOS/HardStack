#!/bin/ash

insmod /lib/modules/$(uname -r)/extra/BiscuitOS-Compact-kernel-default.ko
BiscuitOS-Compact-userspace-default
