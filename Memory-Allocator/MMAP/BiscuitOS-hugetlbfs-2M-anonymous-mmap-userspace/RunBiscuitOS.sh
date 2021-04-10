#!/bin/ash
# mount hugetlbfs
mkdir -p /mnt/BiscuitOS-hugetlbfs/
mount -t hugetlbfs none /mnt/BiscuitOS-hugetlbfs/ -o pagesize=2048K
echo 20 > /proc/sys/vm/nr_hugepages
BiscuitOS-hugetlbfs-2M-anonymous-mmap-userspace-default
