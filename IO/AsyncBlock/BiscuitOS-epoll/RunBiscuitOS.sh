#!/bin/ash

insmod /lib/modules/$(uname -r)/extra/BiscuitOS-epoll-default.ko

BiscuitOS-epoll &
