#!/bin/ash
# mount hugetlbfs
mkdir -p /mnt/BiscuitOS-hugetlbfs/
echo 10 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
mount -t hugetlbfs none /mnt/BiscuitOS-hugetlbfs/ -o pagesize=2048K
BiscuitOS-hugetlb-file-private-mapping-2MiB-default
