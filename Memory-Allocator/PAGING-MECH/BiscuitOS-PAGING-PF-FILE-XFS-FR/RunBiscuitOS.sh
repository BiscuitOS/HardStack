#!/bin/ash
# 
# Check XFS-fs
if ! mount | grep vdq | grep -q xfs; then
   echo "XFS-FS Don't mount"
   exit
fi

# FAULT AROUND
# echo 12288 > /sys/kernel/debug/fault_around_bytes
# Running program
BiscuitOS-PAGING-PF-FILE-XFS-FR-default
