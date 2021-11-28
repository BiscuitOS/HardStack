#!/bin/ash
# 

echo 10 > /proc/sys/vm/nr_hugepages
BiscuitOS-hugetlb-anonymous-share-mapping-shmem-default-Server &
sleep 1
BiscuitOS-hugetlb-anonymous-share-mapping-shmem-default-Client &
sleep 2

# Information for default hugepage pool
cat /proc/meminfo | grep Huge
