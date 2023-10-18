#!/bin/ash
# 
# Check BFS
if ! mount | grep vdi | grep -q bfs; then
   echo "BFS Don't mount"
   exit
fi

# FAULT AROUND
# echo 12288 > /sys/kernel/debug/fault_around_bytes
# Running program
BiscuitOS-PAGING-PF-FILE-BFS-FR-default
