#!/bin/ash
# 
# Check BTRFS
if ! mount | grep vdn | grep -q btrfs; then
   echo "BTRFS Don't mount"
   exit
fi
# Running program
BiscuitOS-PAGING-PF-FILE-BTRFS-default
