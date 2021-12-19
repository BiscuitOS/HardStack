#!/bin/ash
# 
# Allocate 10 Hugepage to 4MiB hugepage pool
echo 10 > /sys/kernel/mm/hugepages/hugepages-4096kB/nr_hugepages
# Running program
BiscuitOS-hugetlb-anonymous-share-mapping-4MiB-default &
sleep 1
# Information for 4MiB hugepage pool
nr_hugepage=$(cat /sys/kernel/mm/hugepages/hugepages-4096kB/nr_hugepages)
free_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-4096kB/free_hugepages)
resv_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-4096kB/resv_hugepages)
surplus_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-4096kB/surplus_hugepages)
echo "BiscuitOS 4MiB Hugepages"
echo " HugePages_Total:  ${nr_hugepage}"
echo " HugePages_Free:   ${free_hugepages}"
echo " HugePages_Rsvd:   ${resv_hugepages}"
echo " HugePages_Surp:   ${surplus_hugepages}"


