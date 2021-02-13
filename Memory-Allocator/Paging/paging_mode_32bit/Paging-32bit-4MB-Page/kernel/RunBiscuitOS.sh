#!/bin/ash

insmod /lib/modules/$(uname -r)/extra/X86-Paging-32bit-4M-Page-kernel-default.ko
X86-Paging-32bit-4M-Page-userspace-default
