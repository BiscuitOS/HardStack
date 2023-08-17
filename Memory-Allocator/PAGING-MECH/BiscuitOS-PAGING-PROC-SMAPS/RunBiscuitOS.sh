#!/bin/ash
# 
# Running program
BiscuitOS-PAGING-PROC-SMAPS-default &
sleep 0.2
PID=$(pidof "BiscuitOS-PAGING-PROC-SMAPS-default")
echo "cat /proc/${PID}/smaps"
echo ""
cat /proc/${PID}/smaps | grep "6000000000" -A 24
