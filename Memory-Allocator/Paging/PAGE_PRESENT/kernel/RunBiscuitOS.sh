#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/PTE-PAGE_PRESENT-kernel-default.ko
PTE-PAGE_PRESENT-userspace-default
