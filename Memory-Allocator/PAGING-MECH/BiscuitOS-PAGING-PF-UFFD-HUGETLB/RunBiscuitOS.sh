#!/bin/ash
# 
# Alloc HUGELTB to Pool
echo 2 > /proc/sys/vm/nr_hugepages
# Running program
BiscuitOS-PAGING-PF-UFFD-HUGETLB-default
