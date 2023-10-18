#!/bin/ash
# 
# Check GFS2-fs
if ! mount | grep vdr | grep -q gfs2; then
   echo "GFS2-FS Don't mount"
   exit
fi

# FAULT AROUND
# echo 12288 > /sys/kernel/debug/fault_around_bytes
# Running program
BiscuitOS-PAGING-PF-FILE-GFS2-FR-default
