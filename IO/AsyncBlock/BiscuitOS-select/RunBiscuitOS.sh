#!/bin/ash

insmod /lib/modules/$(uname -r)/extra/BiscuitOS-select-default.ko

BiscuitOS-select &
