#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-PAGING-PFNMAP-CUSTOMIZE-PREALLOC-default.ko
APP &
