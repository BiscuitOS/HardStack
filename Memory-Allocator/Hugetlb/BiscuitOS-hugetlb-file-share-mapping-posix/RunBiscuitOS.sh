#!/bin/ash
# 
# Mount /dev/shm
mkdir -p /mnt/BiscuitOS-Hugetlbfs/
mount -t hugetlbfs none /mnt/BiscuitOS-Hugetlbfs/

echo 10 > /proc/sys/vm/nr_hugepages
BiscuitOS-hugetlb-file-share-mapping-posix-default-Server &
sleep 1
BiscuitOS-hugetlb-file-share-mapping-posix-default-Client &
sleep 1

# Information for default hugepage pool
cat /proc/meminfo | grep Huge
