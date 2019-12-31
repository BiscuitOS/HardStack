#!/bin/sh

# CMA
#
# (C) 2019.09.24 BiscuitOS <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

lsmod | grep "cma"
[ $? = 1 ] && insmod /lib/modules/5.0.0/extra/cma.ko

# show bitmap
cd /sys/bus/platform/devices/BiscuitOS_CMA
cat bitmap

# Running example to allocate/free CMA
CMA_area_app-0.0.1
