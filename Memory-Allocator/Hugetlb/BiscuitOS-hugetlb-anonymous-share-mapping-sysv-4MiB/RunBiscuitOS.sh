#!/bin/ash
# 

echo 10 > /sys/kernel/mm/hugepages/hugepages-4096kB/nr_hugepages
BiscuitOS-hugetlb-anonymous-share-mapping-sysv-4MiB-default-Server &
sleep 1
BiscuitOS-hugetlb-anonymous-share-mapping-sysv-4MiB-default-Client &
sleep 2

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
