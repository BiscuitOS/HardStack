#!/bin/ash
# 
echo 1 > /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages
nr_hugepage=$(cat /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages)
echo "1Gig: ${nr_hugepage}"
sleep 0.2

# Running program
BiscuitOS-PAGING-PROC-PageMap-1G-default
