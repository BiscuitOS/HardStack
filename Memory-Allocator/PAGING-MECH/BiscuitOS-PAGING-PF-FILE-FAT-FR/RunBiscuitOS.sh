#!/bin/ash
# 
# Check FAT-fs
if ! mount | grep vdg | grep -q vfat; then
   echo "FAT-FS Don't mount"
   exit
fi

# FAULT AROUND
# echo 12288 > /sys/kernel/debug/fault_around_bytes
# Running program
BiscuitOS-PAGING-PF-FILE-FAT-FR-default
