#!/bin/ash
# 
# Check minix-fs
if ! mount | grep vde | grep -q minix; then
   echo "MINIX-FS Don't mount"
   exit
fi

# FAULT AROUND
# echo 12288 > /sys/kernel/debug/fault_around_bytes
# Running program
BiscuitOS-PAGING-PF-FILE-MINIXFS-FR-default
