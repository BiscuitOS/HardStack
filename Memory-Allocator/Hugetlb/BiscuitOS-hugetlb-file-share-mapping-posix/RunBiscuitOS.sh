#!/bin/ash
# 
# Mount /dev/shm
mount | grep "/dev/shm" > /dev/null
[ $? -ne 0 ] && mkdir /dev/shm && mount -t hugetlbfs none /dev/shm

echo 10 > /proc/sys/vm/nr_hugepages
BiscuitOS-hugetlb-file-share-mapping-posix-default-Server
sleep 1
BiscuitOS-hugetlb-file-share-mapping-posix-default-Client
sleep 1

# Information for default hugepage pool
cat /proc/meminfo | grep Huge
