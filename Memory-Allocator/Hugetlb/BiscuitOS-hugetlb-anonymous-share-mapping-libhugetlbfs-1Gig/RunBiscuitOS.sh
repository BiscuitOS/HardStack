#!/bin/ash

# Allocate 10 Hugepage to 1GiB hugepage pool
echo 10 > /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages
# mount 1GiB hugepage spool
mkdir -p /mnt/BiscuitOS-hugetlbfs-1G
mount -t hugetlbfs none -opagesize=1G /mnt/BiscuitOS-hugetlbfs-1G
# Running program
HUGETLB_MORECORE=1G LD_PRELOAD=/lib/libhugetlbfs.so BiscuitOS-hugetlb-anonymous-share-mapping-libhugetlbfs-1Gig-default &
sleep 1
# Information for 1GiB hugepage pool
nr_hugepage=$(cat /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages)
free_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-1048576kB/free_hugepages)
resv_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-1048576kB/resv_hugepages)
surplus_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-1048576kB/surplus_hugepages)
echo "BiscuitOS 1GiB Hugepages"
echo " HugePages_Total:  ${nr_hugepage}"
echo " HugePages_Free:   ${free_hugepages}"
echo " HugePages_Rsvd:   ${resv_hugepages}"
echo " HugePages_Surp:   ${surplus_hugepages}"


