#!/bin/ash
# 

echo 10 > /sys/kernel/mm/hugepages/hugepages-32768kB/nr_hugepages
BiscuitOS-hugetlb-anonymous-share-mapping-memfd-32MiB-default-Server &
sleep 1
BiscuitOS-hugetlb-anonymous-share-mapping-memfd-32MiB-default-Client &
sleep 1
# Information for 32MiB hugepage pool
nr_hugepage=$(cat /sys/kernel/mm/hugepages/hugepages-32768kB/nr_hugepages)
free_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-32768kB/free_hugepages)
resv_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-32768kB/resv_hugepages)
surplus_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-32768kB/surplus_hugepages)
echo "BiscuitOS 32MiB Hugepages"
echo " HugePages_Total:  ${nr_hugepage}"
echo " HugePages_Free:   ${free_hugepages}"
echo " HugePages_Rsvd:   ${resv_hugepages}"
echo " HugePages_Surp:   ${surplus_hugepages}"

