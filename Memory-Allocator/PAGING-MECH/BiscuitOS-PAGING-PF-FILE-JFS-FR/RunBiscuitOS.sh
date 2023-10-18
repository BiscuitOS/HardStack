#!/bin/ash
# 
# Check JFS-fs
if ! mount | grep vdp | grep -q jfs; then
   echo "JFS-FS Don't mount"
   exit
fi

# FAULT AROUND
# echo 12288 > /sys/kernel/debug/fault_around_bytes
# Running program
BiscuitOS-PAGING-PF-FILE-JFS-FR-default
