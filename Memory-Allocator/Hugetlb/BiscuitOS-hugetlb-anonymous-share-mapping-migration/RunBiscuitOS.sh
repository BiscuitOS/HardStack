#!/bin/ash
# 
# Allocate 10 Hugepage to default hugepage pool
echo 10 > /proc/sys/vm/nr_hugepages
# Running program
BiscuitOS-hugetlb-anonymous-share-mapping-migration-default &
# Information for default hugepage pool
cat /sys/devices/system/node/node0/meminfo | grep Huge
cat /sys/devices/system/node/node1/meminfo | grep Huge
sleep 25
cat /sys/devices/system/node/node0/meminfo | grep Huge
cat /sys/devices/system/node/node1/meminfo | grep Huge
