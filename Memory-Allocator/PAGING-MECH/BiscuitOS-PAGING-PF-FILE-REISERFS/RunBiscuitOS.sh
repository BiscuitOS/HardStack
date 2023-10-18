#!/bin/ash
# 
# Check REISERFS
if ! mount | grep vdo | grep -q reiserfs; then
   echo "REISERFS Don't mount"
   exit
fi
# Running program
BiscuitOS-PAGING-PF-FILE-REISERFS-default
