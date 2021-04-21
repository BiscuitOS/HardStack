#!/bin/ash

# Install Module
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-paging-kernel-default.ko
BiscuitOS-special-share-mmap-userfault-default
