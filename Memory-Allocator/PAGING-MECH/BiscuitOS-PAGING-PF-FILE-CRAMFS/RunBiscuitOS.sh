#!/bin/ash
# 
# Check CRAMFS
if ! mount | grep vdj | grep -q cramfs; then
   echo "CRAMFS Don't mount"
   exit
fi
# Running program
BiscuitOS-PAGING-PF-FILE-CRAMFS-default
