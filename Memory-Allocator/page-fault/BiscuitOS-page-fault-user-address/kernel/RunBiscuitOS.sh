#!/bin/ash

insmod /lib/modules/$(uname -r)/extra/BiscuitOS-page-fault-user-address-on-kernel-default.ko
BiscuitOS-page-fault-user-address-on-userspace-default
