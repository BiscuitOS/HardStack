#!/bin/ash
# mount hugetlbfs
mkdir -p /mnt/BiscuitOS-hugetlbfs/
mount -t hugetlbfs none /mnt/BiscuitOS-hugetlbfs/ -o pagesize=2048K
echo 20 > /proc/sys/vm/nr_hugepages
BiscuitOS-anonymous-hugetlbfs-2M-mmap-userspace-default
