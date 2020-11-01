#!/bin/ash
# 
# Establish Specific Private mmap for KVM

insmod /lib/modules/$(uname -r)/extra/BiscuitOS-UKVM-SMAP-kernel.ko
BiscuitOS-UKVM-SMAP-userspace
