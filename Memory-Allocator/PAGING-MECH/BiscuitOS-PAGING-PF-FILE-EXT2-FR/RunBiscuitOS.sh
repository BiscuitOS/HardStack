#!/bin/ash
# 
# Check EXT2-fs
if ! mount | grep vdb | grep -q ext2; then
   echo "EXT2-FS Don't mount"
   exit
fi

# FAULT AROUND
# echo 12288 > /sys/kernel/debug/fault_around_bytes
# Running program
BiscuitOS-PAGING-PF-FILE-EXT2-FR-default
