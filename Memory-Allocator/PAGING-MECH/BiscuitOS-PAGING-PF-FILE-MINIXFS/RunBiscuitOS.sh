#!/bin/ash
# 
# Check minix-fs
if ! mount | grep vde | grep -q minix; then
   echo "MINIX-FS Don't mount"
   exit
fi

# Running program
BiscuitOS-PAGING-PF-FILE-MINIXFS-default
