#!/bin/ash
# 

echo 10 > /proc/sys/vm/nr_hugepages
BiscuitOS-hugetlb-anonymous-share-mapping-memfd-default-Server &
sleep 1
BiscuitOS-hugetlb-anonymous-share-mapping-memfd-default-Client &
sleep 1
cat /proc/meminfo | grep Huge

