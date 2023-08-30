#!/bin/ash
# 
# Running program
echo 4 > /proc/sys/vm/nr_hugepages
BiscuitOS-PAGING-PROC-SMAPS-SHARED-HUGETLB-default &
sleep 0.2
_PID=$(pidof "BiscuitOS-PAGING-PROC-SMAPS-SHARED-HUGETLB-default")
PID=$(echo "${_PID}" | awk '{print $1}')
echo "cat /proc/${PID}/smaps"
echo ""
cat /proc/${PID}/smaps | grep "6000000000" -A 23
