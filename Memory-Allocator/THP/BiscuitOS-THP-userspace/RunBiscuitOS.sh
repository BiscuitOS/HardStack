#!/bin/ash
# 
# Setup scan sleep
echo 3000 > /sys/kernel/mm/transparent_hugepage/khugepaged/scan_sleep_millisecs
# Running program
BiscuitOS-THP-userspace-default &
