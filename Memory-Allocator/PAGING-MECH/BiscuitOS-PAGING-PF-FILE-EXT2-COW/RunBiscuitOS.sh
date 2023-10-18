#!/bin/ash
# 
# Check EXT2-fs
if ! mount | grep vdb | grep -q ext2; then
   echo "EXT2-FS Don't mount"
   exit
fi

# Running program
BiscuitOS-PAGING-PF-FILE-EXT2-COW-default
