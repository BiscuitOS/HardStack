#!/bin/ash
# 
# Allocate 1 Hugepage to 1Gig hugepage pool
echo 1 > /proc/sys/vm/nr_hugepages
sleep 0.1
# Running program
BiscuitOS-CACHE-PERFORMANCE-PREFETCH-default &


