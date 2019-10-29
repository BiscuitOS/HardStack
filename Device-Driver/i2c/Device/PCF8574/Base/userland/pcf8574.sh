#!/bin/bash

# PCF8574A
#
# (C) 2019.09.24 BiscuitOS <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

i2cdetect -y 1

[ -f /usr/bin/LDD_pcf8574_app-0.0.1 ] && LDD_pcf8574_app-0.0.1
