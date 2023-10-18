#!/bin/ash
# 
echo always > /sys/kernel/mm/transparent_hugepage/enabled
# Running program
BiscuitOS-PAGING-HUGE-PF-COW-ANON-THP-default
