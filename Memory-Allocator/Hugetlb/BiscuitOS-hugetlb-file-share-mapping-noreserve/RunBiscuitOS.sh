#!/bin/ash
# mount hugetlbfs
mkdir -p /mnt/BiscuitOS-hugetlbfs/
echo 10 > /proc/sys/vm/nr_hugepages
mount -t hugetlbfs none /mnt/BiscuitOS-hugetlbfs/
BiscuitOS-hugetlb-file-share-mapping-noreserve-default &
cat /proc/meminfo | grep Huge

