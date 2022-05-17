#!/bin/ash

insmod /lib/modules/$(uname -r)/extra/BiscuitOS-poll-default.ko

BiscuitOS-poll &
