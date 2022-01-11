#!/bin/ash

# Allocate 10 Hugepage to 32MiB hugepage pool
echo 10 > /sys/kernel/mm/hugepages/hugepages-32768kB/nr_hugepages
# mount 32MiB hugepage spool
mkdir -p /mnt/BiscuitOS-hugetlbfs-32M
mount -t hugetlbfs none -o pagesize=32M /mnt/BiscuitOS-hugetlbfs-32M
# Running program
HUGETLB_MORECORE=32M LD_PRELOAD=/lib/libhugetlbfs.so BiscuitOS-hugetlb-anonymous-share-mapping-libhugetlbfs-32MiB-default &
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


