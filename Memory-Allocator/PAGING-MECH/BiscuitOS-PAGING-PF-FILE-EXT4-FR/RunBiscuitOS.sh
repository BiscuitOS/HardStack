#!/bin/ash
# 
# Check EXT4-fs
if ! mount | grep vdd | grep -q ext4; then
   echo "EXT4-FS Don't mount"
   exit
fi

# FAULT AROUND
# echo 12288 > /sys/kernel/debug/fault_around_bytes
# Running program
BiscuitOS-PAGING-PF-FILE-EXT4-FR-default
