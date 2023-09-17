#!/bin/ash
# 
# Running program
BiscuitOS-PAGING-PF-ANON-KSM-default &
sleep 0.5
# Enable KSM
echo 1 > /sys/kernel/mm/ksm/run
