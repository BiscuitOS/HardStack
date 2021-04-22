#!/bin/ash

# Install Module
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-special-private-mmap-kernel-default.ko
BiscuitOS-special-private-mmap-userspace-default
