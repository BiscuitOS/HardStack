#!/bin/ash

# Install Module
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-MMAP-LazyAlloc-VMALLOC32-default.ko
BiscuitOS-MMAP-Alloc-Special-Memory-Userspace-default
