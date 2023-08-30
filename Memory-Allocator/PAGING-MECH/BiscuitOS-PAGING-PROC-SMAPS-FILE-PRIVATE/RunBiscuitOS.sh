#!/bin/ash
# 

echo "Hello BiscuitOS" > /tmp/BiscuitOS.txt
# Running program
BiscuitOS-PAGING-PROC-SMAPS-FILE-PRIVATE-default &
sleep 0.2
_PID=$(pidof "BiscuitOS-PAGING-PROC-SMAPS-FILE-PRIVATE-default")
PID=$(echo "${_PID}" | awk '{print $1}')
echo "cat /proc/${PID}/smaps"
echo ""
cat /proc/${PID}/smaps | grep "6000000000" -A 24
