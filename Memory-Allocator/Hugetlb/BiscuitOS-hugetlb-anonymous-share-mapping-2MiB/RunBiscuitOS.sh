#!/bin/ash
# 
# Allocate 10 Hugepage to 2MiB hugepage pool
echo 10 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
# Running program
BiscuitOS-hugetlb-anonymous-share-mapping-2MiB-default &
sleep 1
# Information for 2MiB hugepage pool
nr_hugepage=$(cat /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages)
free_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-2048kB/free_hugepages)
resv_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-2048kB/resv_hugepages)
surplus_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-2048kB/surplus_hugepages)
echo "BiscuitOS 2MiB Hugepages"
echo " HugePages_Total:  ${nr_hugepage}"
echo " HugePages_Free:   ${free_hugepages}"
echo " HugePages_Rsvd:   ${resv_hugepages}"
echo " HugePages_Surp:   ${surplus_hugepages}"


