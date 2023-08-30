#!/bin/ash
# 
# Running program
BiscuitOS-PAGING-PROC-SMAPS-LOCKED-default &
sleep 0.2
_PID=$(pidof "BiscuitOS-PAGING-PROC-SMAPS-LOCKED-default")
PID=$(echo "${_PID}" | awk '{print $1}')
echo "cat /proc/${PID}/smaps"
echo ""
cat /proc/${PID}/smaps | grep "6000000000" -A 23
