#!/bin/ash
# 
# Enable THP always
echo madvise > /sys/kernel/mm/transparent_hugepage/enabled
# Running program
BiscuitOS-THP-AnonHugepage-madvise-default &
