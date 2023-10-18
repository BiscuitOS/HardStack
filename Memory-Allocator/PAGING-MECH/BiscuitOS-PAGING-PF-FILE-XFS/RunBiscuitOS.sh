#!/bin/ash
# 
# Check XFS-fs
if ! mount | grep vdq | grep -q xfs; then
   echo "XFS-FS Don't mount"
   exit
fi
# Running program
BiscuitOS-PAGING-PF-FILE-XFS-default
