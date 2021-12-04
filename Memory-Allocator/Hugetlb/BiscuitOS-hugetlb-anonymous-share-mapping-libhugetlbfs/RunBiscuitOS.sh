#!/bin/ash

# Default Hugepage pool
echo 10 > /proc/sys/vm/nr_hugepages
# Usage
HUGETLB_MORECORE=yes LD_PRELOAD=/lib/libhugetlbfs.so BiscuitOS-hugetlb-anonymous-share-mapping-libhugetlbfs-default &
sleep 1
cat /proc/meminfo | grep Huge

