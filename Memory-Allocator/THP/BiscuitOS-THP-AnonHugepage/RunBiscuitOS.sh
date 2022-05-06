#!/bin/ash
# 
# Enable THP always
echo always > /sys/kernel/mm/transparent_hugepage/enabled
# Running program
BiscuitOS-THP-AnonHugepage-default &
