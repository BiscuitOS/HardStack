#!/bin/ash
# 
# Check SQUASHFS
if ! mount | grep vdm | grep -q squashfs; then
   echo "SQUASHFS Don't mount"
   exit
fi
# Running program
BiscuitOS-PAGING-PF-FILE-SQUASHFS-default
