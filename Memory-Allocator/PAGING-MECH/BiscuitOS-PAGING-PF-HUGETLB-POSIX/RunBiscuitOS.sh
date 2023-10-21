#!/bin/ash
# 
# Allocate 10 Hugepage to default hugepage pool
echo 10 > /proc/sys/vm/nr_hugepages
mkdir -p /mnt/BiscuitOS-Hugetlbfs/
mount -t hugetlbfs none /mnt/BiscuitOS-Hugetlbfs/
# Running program
BiscuitOS-PAGING-PF-HUGETLB-POSIX-default-SERVER &
sleep 0.5
BiscuitOS-PAGING-PF-HUGETLB-POSIX-default-CLIENT &
