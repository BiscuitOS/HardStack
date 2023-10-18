#!/bin/ash
# 
echo always > /sys/kernel/mm/transparent_hugepage/enabled
# Running program
BiscuitOS-PAGING-HUGE-PF-ZSWAP-ANON-THP-default
