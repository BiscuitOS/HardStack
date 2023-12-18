#!/bin/ash
# mount hugetlbfs
mkdir -p /mnt/BiscuitOS-hugetlbfs/
mount -t hugetlbfs none /mnt/BiscuitOS-hugetlbfs/ -o pagesize=1G
echo 2 > /proc/sys/vm/nr_hugepages
BiscuitOS-hugetlbfs-1G-anonymous-share-mmap-userspace-default
