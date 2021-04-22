#!/bin/ash

# Install Module
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-special-share-mmap-kernel-default.ko
BiscuitOS-special-share-mmap-userspace-default
