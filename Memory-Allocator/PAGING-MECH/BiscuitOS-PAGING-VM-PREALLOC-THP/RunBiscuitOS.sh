#!/bin/ash
# 
# Enable THP
echo always > /sys/kernel/mm/transparent_hugepage/enabled

# Running program
BiscuitOS-PAGING-VM-PREALLOC-THP-default
