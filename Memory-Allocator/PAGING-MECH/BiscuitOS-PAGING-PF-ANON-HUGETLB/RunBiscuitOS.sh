#!/bin/ash
# 
echo 2 > /proc/sys/vm/nr_hugepages
# Running program
BiscuitOS-PAGING-PF-ANON-HUGETLB-default
