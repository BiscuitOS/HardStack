#!/bin/ash
# 
# Check VFAT-fs
if ! mount | grep vdf | grep -q vfat; then
   echo "VFAT-FS Don't mount"
   exit
fi
# Running program
BiscuitOS-PAGING-PF-FILE-VFAT-default
