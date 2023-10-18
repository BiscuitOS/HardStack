#!/bin/ash
# 
# Check MSDOS-fs
if ! mount | grep vdh | grep -q vfat; then
   echo "MSDOS-FS Don't mount"
   exit
fi
# Running program
BiscuitOS-PAGING-PF-FILE-MSDOS-default
