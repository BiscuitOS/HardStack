#!/bin/ash
# 
# Running program
BiscuitOS-MEMORY-MMAP-AREA-FIXED-default &
sleep 0.1

# MAPS
PID=$(pidof "BiscuitOS-MEMORY-MMAP-AREA-FIXED-default")
echo ""
echo ""
echo "MAP: /proc/${PID}/maps"
cat /proc/${PID}/maps

