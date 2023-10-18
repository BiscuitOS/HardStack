#!/bin/ash
# 
# Check MSDOS-fs
if ! mount | grep vdh | grep -q vfat; then
   echo "MSDOS-FS Don't mount"
   exit
fi

# FAULT AROUND
# echo 12288 > /sys/kernel/debug/fault_around_bytes
# Running program
BiscuitOS-PAGING-PF-FILE-MSDOS-FR-default
