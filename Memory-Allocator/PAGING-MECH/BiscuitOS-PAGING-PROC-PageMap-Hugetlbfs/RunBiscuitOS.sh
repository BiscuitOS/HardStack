#!/bin/ash
# 
echo 1 > /proc/sys/vm/nr_hugepages

# Running program
BiscuitOS-PAGING-PROC-PageMap-Hugetlbfs-default
