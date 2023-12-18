#!/bin/ash

# Install Module
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-MMAP-PreAlloc-Special-ZONE-NODE-default.ko
BiscuitOS-MMAP-Alloc-Special-Memory-Userspace-default
