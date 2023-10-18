#!/bin/ash
# 
# Check BFS
if ! mount | grep vdi | grep -q bfs; then
   echo "BFS Don't mount"
   exit
fi
# Running program
BiscuitOS-PAGING-PF-FILE-BFS-default
