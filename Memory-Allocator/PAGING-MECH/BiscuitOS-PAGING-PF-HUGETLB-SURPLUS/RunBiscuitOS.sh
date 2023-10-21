#!/bin/ash
# 
# Allocate 1 Surplus Hugepage to default hugepage pool
echo 1 > /proc/sys/vm/nr_overcommit_hugepages
# Running program
BiscuitOS-PAGING-PF-HUGETLB-SURPLUS-default
