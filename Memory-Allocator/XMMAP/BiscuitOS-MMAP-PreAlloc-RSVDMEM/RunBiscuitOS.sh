#!/bin/ash

# Install Module
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-MMAP-PreAlloc-RSVDMEM-default.ko
BiscuitOS-MMAP-Alloc-Special-Memory-Userspace-default
