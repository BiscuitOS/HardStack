#!/bin/ash
# 
# Allocate 10 Hugepage to default hugepage pool
echo 10 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
# Running program
BiscuitOS-hugetlb-anonymous-share-mapping-default
# Information for default hugepage pool
cat /proc/meminfo | grep Huge
