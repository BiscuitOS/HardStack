#!/bin/ash
# 
echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo  1 > /sys/kernel/mm/transparent_hugepage/use_zero_page
# Running program
BiscuitOS-PAGING-HUGE-PF-RO-ANON-THP-default
