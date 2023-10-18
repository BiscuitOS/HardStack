#!/bin/ash
# 
# Check EXT3-fs
if ! mount | grep vdc | grep -q ext3; then
   echo "EXT3-FS Don't mount"
   exit
fi

# Running program
BiscuitOS-PAGING-PF-FILE-EXT3-COW-default
