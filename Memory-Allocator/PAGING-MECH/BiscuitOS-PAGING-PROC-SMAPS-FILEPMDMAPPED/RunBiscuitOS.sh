#!/bin/ash
# 

# Running program
APP &
sleep 0.2
_PID=$(pidof "APP")
PID=$(echo "${_PID}" | awk '{print $1}')
echo "cat /proc/${PID}/smaps"
echo ""
cat /proc/${PID}/smaps | grep "BiscuitOS-PageTable" -A 24
