#!/bin/ash
# 
# Check JFS-fs
if ! mount | grep vdp | grep -q jfs; then
   echo "JFS-FS Don't mount"
   exit
fi
# Running program
BiscuitOS-PAGING-PF-FILE-JFS-default
