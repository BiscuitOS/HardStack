#!/bin/ash
# 
mkdir -p /mnt/DAX
mkfs.ext4 -b 4096 /dev/pmem0
mount -o dax=always /dev/pmem0 /mnt/DAX/
echo always > /sys/kernel/mm/transparent_hugepage/enabled
dd bs=1M count=4 if=/dev/zero of=/mnt/DAX/BiscuitOS.txt > /dev/null 2>&1

echo ""
echo "Running Programs"
# Running program
BiscuitOS-PAGING-PROC-SMAPS-default &
sleep 0.2
_PID=$(pidof "BiscuitOS-PAGING-PROC-SMAPS-default")
PID=$(echo "${_PID}" | awk '{print $1}')
echo "cat /proc/${PID}/smaps"
echo ""
cat /proc/${PID}/smaps | grep "6000000000" -A 24
