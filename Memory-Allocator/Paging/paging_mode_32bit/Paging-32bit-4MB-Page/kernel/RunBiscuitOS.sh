#!/bin/ash

insmod /lib/modules/$(uname -r)/extra/X86-Paging-32bit-4K-Page-kernel-default.ko
X86-Paging-32bit-4K-Page-userspace-default
