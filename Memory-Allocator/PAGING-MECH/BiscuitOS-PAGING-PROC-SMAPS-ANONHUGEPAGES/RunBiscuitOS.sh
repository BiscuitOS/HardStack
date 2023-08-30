#!/bin/ash
# 
echo always > /sys/kernel/mm/transparent_hugepage/enabled

# Running program
BiscuitOS-PAGING-PROC-SMAPS-ANONHUGEPAGES-default &
sleep 0.2
_PID=$(pidof "BiscuitOS-PAGING-PROC-SMAPS-ANONHUGEPAGES-default")
PID=$(echo "${_PID}" | awk '{print $1}')
echo "cat /proc/${PID}/smaps"
echo ""
cat /proc/${PID}/smaps | grep "6000000000" -A 24
