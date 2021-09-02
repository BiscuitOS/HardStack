#!/bin/ash

# Reserved hugepage 
echo 4 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
# Usage
HUGETLB_MORECORE=yes LD_PRELOAD=/lib/libhugetlbfs.so BiscuitOS-libhugetlbfs-usage-default

