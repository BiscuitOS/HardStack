#!/bin/bash

# I2C Device
#
# (C) 2019.09.24 BiscuitOS <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

i2cdetect -y 1

[ -f /usr/bin/LDD_at24c08_app-0.0.1 ] && LDD_at24c08_app-0.0.1
