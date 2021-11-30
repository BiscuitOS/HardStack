#!/bin/ash
# 
# Allocate 10 Hugepage to default hugepage pool
echo 10 > /proc/sys/vm/nr_hugepages
# Running program
BiscuitOS-hugetlb-anonymous-share-mapping-populate-default
# Information for default hugepage pool
cat /proc/meminfo | grep Huge
