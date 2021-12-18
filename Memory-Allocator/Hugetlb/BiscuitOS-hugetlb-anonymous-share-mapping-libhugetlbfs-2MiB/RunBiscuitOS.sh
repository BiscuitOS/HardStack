#!/bin/ash

# Allocate 10 Hugepage to 2MiB hugepage pool
echo 10 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
# mount 2MiB hugepage spool
mkdir -p /mnt/BiscuitOS-hugetlbfs-2M
mount -t hugetlbfs none -opagesize=2M /mnt/BiscuitOS-hugetlbfs-2M
# Running program
HUGETLB_MORECORE=2M LD_PRELOAD=/lib/libhugetlbfs.so BiscuitOS-hugetlb-anonymous-share-mapping-libhugetlbfs-2MiB-default &
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


