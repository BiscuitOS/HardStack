#!/bin/ash
# 

mkdir -p /mnt/BiscuitOS-hugetlbfs/
mount -t hugetlbfs none /mnt/BiscuitOS-hugetlbfs/
echo 10 > /proc/sys/vm/nr_hugepages
BiscuitOS-hugetlb-file-share-mapping-usock-default-Server &
sleep 1
BiscuitOS-hugetlb-file-share-mapping-usock-default-Client &
sleep 1
cat /proc/meminfo | grep Huge

