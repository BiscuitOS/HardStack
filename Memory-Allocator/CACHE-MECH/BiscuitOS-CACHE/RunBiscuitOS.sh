#!/bin/ash
# 
# Allocate 1 Hugepage to 1Gig hugepage pool
echo 1 > /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages
sleep 0.1
# Running program
BiscuitOS-CACHE-default &


