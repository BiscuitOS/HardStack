#!/bin/ash

insmod /lib/modules/$(uname -r)/extra/BiscuitOS-Paging-4K-Page-kernel-default.ko
BiscuitOS-Paging-4K-Page-userspace-default
