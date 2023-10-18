#!/bin/ash
# 
# Check VFAT-fs
if ! mount | grep vdf | grep -q vfat; then
   echo "VFAT-FS Don't mount"
   exit
fi

# FAULT AROUND
# echo 12288 > /sys/kernel/debug/fault_around_bytes
# Running program
BiscuitOS-PAGING-PF-FILE-VFAT-FR-default
