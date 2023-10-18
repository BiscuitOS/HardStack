#!/bin/ash
# 
# Check BTRFS
if ! mount | grep vdn | grep -q btrfs; then
   echo "BTRFS Don't mount"
   exit
fi

# FAULT AROUND
# echo 12288 > /sys/kernel/debug/fault_around_bytes
# Running program
BiscuitOS-PAGING-PF-FILE-BTRFS-FR-default
