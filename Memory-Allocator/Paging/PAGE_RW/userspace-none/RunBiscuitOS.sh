#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/PTE-PAGE_RW-kernel-default.ko
PTE-PAGE_RW-userspace-none-default
