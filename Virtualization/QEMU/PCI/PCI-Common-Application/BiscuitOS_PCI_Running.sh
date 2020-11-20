#!/bin/ash
  
# This is scripts for using PCI-edu
# You can use it when starting a KVM guest with bridge mode network.
# set your bridge name
#
# (C) 2020.10.24 BiscuitOS <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

# Modules install
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-QEMU-PCI-module-0.0.1.ko

# Identifiction: just returns some fixed magic bytes.
dd bs=4 status=none if=/dev/BiscuitOS_PCI count=1 skip=0 | od -An -t x1
# => 010000ed

# Negator. Sanity check that the hardware is getting updated.
dd bs=4 status=none if=/dev/BiscuitOS_PCI count=1 skip=1 | od -An -t x1
printf '\xF0\xF0\xF0\xF0' | dd bs=4 status=none of=/dev/BiscuitOS_PCI count=1 seek=1
dd bs=4 status=none if=/dev/BiscuitOS_PCI count=1 skip=1 | od -An -t x1
# => 0F0F0F0F

# Factorial calculator.
# factorial(0xC) = 0x1c8cfc00
printf '\x0C\x00\x00\x00' | dd bs=4 status=none of=/dev/BiscuitOS_PCI count=1 seek=2
sleep 1
dd bs=4 status=none if=/dev/BiscuitOS_PCI count=1 skip=2 | od -An -t x1
dd bs=4 status=none if=/dev/BiscuitOS_PCI count=1 skip=8 | od -An -t x1
# => 1c8cfc00

# Manual IRQ raising.
printf '\x04\x03\x02\x01' | dd bs=4 status=none of=/dev/BiscuitOS_PCI count=1 seek=24
# => interrupt
sleep 1
printf '\x08\x07\x06\x05' | dd bs=4 status=none of=/dev/BiscuitOS_PCI count=1 seek=24
# => interrupt
sleep 1
