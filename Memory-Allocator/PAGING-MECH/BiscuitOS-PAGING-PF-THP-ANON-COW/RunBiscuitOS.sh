#!/bin/ash
# 
echo always > /sys/kernel/mm/transparent_hugepage/enabled
# Running program
BiscuitOS-PAGING-PF-THP-ANON-COW-default &
