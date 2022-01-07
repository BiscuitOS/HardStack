#!/bin/ash
# 
# Allocate 10 Hugepage to default hugepage pool
echo 2 > /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages
# Running program
BiscuitOS-hugetlb-anonymous-share-mapping-migration-1Gig-default &
# Information for default hugepage pool
sleep 2
nr_hugepage=$(cat /sys/devices/system/node/node0/hugepages/hugepages-1048576kB/nr_hugepages)
free_hugepages=$(cat /sys/devices/system/node/node0/hugepages/hugepages-1048576kB/free_hugepages)
surplus_hugepages=$(cat /sys/devices/system/node/node0/hugepages/hugepages-1048576kB/surplus_hugepages)
echo "BiscuitOS 1Gig Hugepages NUMA NODE 0"
echo " HugePages_Total:  ${nr_hugepage}"
echo " HugePages_Free:   ${free_hugepages}"
echo " HugePages_Surp:   ${surplus_hugepages}"
sleep 25
nr_hugepage=$(cat /sys/devices/system/node/node1/hugepages/hugepages-1048576kB/nr_hugepages)
free_hugepages=$(cat /sys/devices/system/node/node1/hugepages/hugepages-1048576kB/free_hugepages)
surplus_hugepages=$(cat /sys/devices/system/node/node1/hugepages/hugepages-1048576kB/surplus_hugepages)
echo "BiscuitOS 1Gig Hugepages NUMA NODE 1"
echo " HugePages_Total:  ${nr_hugepage}"
echo " HugePages_Free:   ${free_hugepages}"
echo " HugePages_Surp:   ${surplus_hugepages}"
