#!/bin/ash
# 
# Running program
BiscuitOS-MEMORY-MMAP-AREA-NOREPLACE-default &
sleep 0.1

# MAPS
PID=$(pidof "BiscuitOS-MEMORY-MMAP-AREA-NOREPLACE-default")
echo ""
echo ""
echo "MAP: /proc/${PID}/maps"
cat /proc/${PID}/maps

