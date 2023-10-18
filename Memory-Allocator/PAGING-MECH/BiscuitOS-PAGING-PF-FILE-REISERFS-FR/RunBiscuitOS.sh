#!/bin/ash
# 
# Check REISERFS
if ! mount | grep vdo | grep -q reiserfs; then
   echo "REISERFS Don't mount"
   exit
fi

# FAULT AROUND
# echo 12288 > /sys/kernel/debug/fault_around_bytes
# Running program
BiscuitOS-PAGING-PF-FILE-REISERFS-FR-default
