#!/bin/ash
# 
# Establish HugePage for KVM

mkdir -p /mnt/HugePagefs
mount none /mnt/HugePagefs -t hugetlbfs

echo 20 > /proc/sys/vm/nr_hugepages
BiscuitOS-UKVM-2M-default
