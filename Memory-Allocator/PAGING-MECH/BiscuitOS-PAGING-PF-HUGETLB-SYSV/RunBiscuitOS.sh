#!/bin/ash
# 
# Allocate 10 Hugepage to default hugepage pool
echo 10 > /proc/sys/vm/nr_hugepages
# Running program
BiscuitOS-PAGING-PF-HUGETLB-SYSV-default-SERVER &
sleep 0.5
BiscuitOS-PAGING-PF-HUGETLB-SYSV-default-CLIENT &
