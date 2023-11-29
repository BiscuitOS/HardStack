#!/bin/ash
# 
mkfs.xfs /dev/pmem0
mkdir -p /mnt/DAX
mount -t xfs -o dax=always /dev/pmem0 /mnt/DAX
dd bs=1M count=4 if=/dev/zero of=/mnt/DAX/BiscuitOS.txt > /dev/null 2>&1

# Running program
BiscuitOS-PAGING-VM-LAZYALLOC-DAX-default
