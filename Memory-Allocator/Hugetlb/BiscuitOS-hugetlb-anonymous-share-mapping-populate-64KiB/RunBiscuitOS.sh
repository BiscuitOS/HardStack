#!/bin/ash
# 
# Allocate 10 Hugepage to 64KiB hugepage pool
echo 10 > /sys/kernel/mm/hugepages/hugepages-64kB/nr_hugepages
# Running program
BiscuitOS-hugetlb-anonymous-share-mapping-populate-64KiB-default &
sleep 1
# Information for 64KiB hugepage pool
nr_hugepage=$(cat /sys/kernel/mm/hugepages/hugepages-64kB/nr_hugepages)
free_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-64kB/free_hugepages)
resv_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-64kB/resv_hugepages)
surplus_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-64kB/surplus_hugepages)
echo "BiscuitOS 64KiB Hugepages"
echo " HugePages_Total:  ${nr_hugepage}"
echo " HugePages_Free:   ${free_hugepages}"
echo " HugePages_Rsvd:   ${resv_hugepages}"
echo " HugePages_Surp:   ${surplus_hugepages}"
