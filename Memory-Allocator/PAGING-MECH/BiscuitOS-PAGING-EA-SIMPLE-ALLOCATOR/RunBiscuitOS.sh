#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-PAGING-EA-SIMPLE-ALLOCATOR-default.ko
APP &
