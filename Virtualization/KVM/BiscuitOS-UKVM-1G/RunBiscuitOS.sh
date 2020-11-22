#!/bin/ash
# 
# Establish HugePage for KVM

mkdir -p /mnt/HugePagefs
mount none /mnt/HugePagefs -t hugetlbfs -o pagesize=1G

echo 1 > /proc/sys/vm/nr_hugepages
BiscuitOS-UKVM-1G-default
