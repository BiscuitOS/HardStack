#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-PAGING-PFNMAP-CUSTOMIZE-LAZYALLOC-default.ko
APP &
