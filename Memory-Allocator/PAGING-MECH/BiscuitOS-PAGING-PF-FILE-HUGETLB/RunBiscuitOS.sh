#!/bin/ash
# 
mkdir -p /mnt/BiscuitOS-hugetlbfs/
mount -t hugetlbfs none /mnt/BiscuitOS-hugetlbfs/ -o pagesize=2048K
echo 2 > /proc/sys/vm/nr_hugepages
# Running program
BiscuitOS-PAGING-PF-FILE-HUGETLB-default
