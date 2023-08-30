#!/bin/ash
# 
# Running program
BiscuitOS-PAGING-PROC-SMAPS-ROLLUP-default &
sleep 0.2
PID=$(pidof "BiscuitOS-PAGING-PROC-SMAPS-ROLLUP-default")
echo "cat /proc/${PID}/smaps_rollup"
echo ""
cat /proc/${PID}/smaps_rollup
