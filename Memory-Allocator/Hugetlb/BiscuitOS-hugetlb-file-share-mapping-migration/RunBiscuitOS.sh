#!/bin/ash
# 
# Allocate 10 Hugepage to default hugepage pool
mkdir -p /mnt/BiscuitOS-hugetlbfs
mount -t hugetlbfs none /mnt/BiscuitOS-hugetlbfs/
numactl -m 0 echo 10 > /proc/sys/vm/nr_hugepages_mempolicy
cat /sys/devices/system/node/node0/meminfo | grep Huge
cat /sys/devices/system/node/node1/meminfo | grep Huge

# Running program
BiscuitOS-hugetlb-file-share-mapping-migration-default &
# Information for default hugepage pool
sleep 2
cat /sys/devices/system/node/node0/meminfo | grep Huge
cat /sys/devices/system/node/node1/meminfo | grep Huge
sleep 25
cat /sys/devices/system/node/node0/meminfo | grep Huge
cat /sys/devices/system/node/node1/meminfo | grep Huge
