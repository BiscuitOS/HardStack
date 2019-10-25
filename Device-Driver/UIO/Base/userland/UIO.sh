#!/bin/bash

# UIO Device
#
# (C) 2019.09.24 BiscuitOS <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

ls /dev/uio0

cat /sys/class/uio/uio0/maps/map0/addr
cat /sys/class/uio/uio0/maps/map0/size
