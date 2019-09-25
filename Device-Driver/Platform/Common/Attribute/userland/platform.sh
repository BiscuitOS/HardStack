#!/bin/sh

# Platform Device attribute
#
# (C) 2019.09.24 BiscuitOS <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

cd /sys/bus/platform/devices/Platform_attr.1/
ls

echo "-------------------------------"
# Hexadecimal
echo 0x567 > Hexadecimal
echo "Hexadecimal: $(cat Hexadecimal)"

# Integer
echo "123445" > Integer
echo "Integer:     $(cat Integer)"

# String
echo "Hello-BiscuitOS" > String
echo "String:      $(cat String)"
echo "-------------------------------"
