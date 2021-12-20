#!/bin/ash
# 

echo 10 > /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages
BiscuitOS-hugetlb-anonymous-share-mapping-memfd-1Gig-default-Server &
sleep 1
BiscuitOS-hugetlb-anonymous-share-mapping-memfd-1Gig-default-Client &
sleep 1
# Information for 1Gig hugepage pool
nr_hugepage=$(cat /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages)
free_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-1048576kB/free_hugepages)
resv_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-1048576kB/resv_hugepages)
surplus_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-1048576kB/surplus_hugepages)
echo "BiscuitOS 1Gig Hugepages"
echo " HugePages_Total:  ${nr_hugepage}"
echo " HugePages_Free:   ${free_hugepages}"
echo " HugePages_Rsvd:   ${resv_hugepages}"
echo " HugePages_Surp:   ${surplus_hugepages}"

