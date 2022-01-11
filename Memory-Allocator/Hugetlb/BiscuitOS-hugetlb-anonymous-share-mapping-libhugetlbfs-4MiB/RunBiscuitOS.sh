#!/bin/ash

# Allocate 10 Hugepage to 4MiB hugepage pool
echo 10 > /sys/kernel/mm/hugepages/hugepages-4096kB/nr_hugepages
# mount 4MiB hugepage spool
mkdir -p /mnt/BiscuitOS-hugetlbfs-4M
mount -t hugetlbfs none -o pagesize=4M /mnt/BiscuitOS-hugetlbfs-4M
# Running program
HUGETLB_MORECORE=4M LD_PRELOAD=/lib/libhugetlbfs.so BiscuitOS-hugetlb-anonymous-share-mapping-libhugetlbfs-4MiB-default &
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


