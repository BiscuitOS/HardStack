#!/bin/ash
# 
# Check F2FS-fs
if ! mount | grep vds | grep -q f2fs; then
   echo "F2FS-FS Don't mount"
   exit
fi

# Running program
BiscuitOS-PAGING-PF-FILE-F2FS-COW-default
