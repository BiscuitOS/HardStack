#!/bin/ash
# 
# Allocate 10 Hugepage to default hugepage pool
echo 1 > /proc/sys/vm/nr_hugepages
# Running program
BiscuitOS-PAGING-PF-HUGETLB-COW-FAULT-default
