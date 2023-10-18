#!/bin/ash
# 
echo always > /sys/kernel/mm/transparent_hugepage/enabled
# Running program
BiscuitOS-PAGING-HUGE-PF-KSM-ANON-THP-default &
sleep 0.5
# Enable KSM
echo 1 > /sys/kernel/mm/ksm/run
