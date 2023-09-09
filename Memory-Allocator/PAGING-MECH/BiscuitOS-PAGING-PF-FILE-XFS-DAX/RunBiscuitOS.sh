#!/bin/ash
# 
# XFS with DAX
mkfs.xfs /dev/pmem0
mkdir -p /mnt/xfs-dax
mount -t xfs -o dax=always /dev/pmem0 /mnt/xfs-dax
dd bs=1M count=4 if=/dev/zero of=/mnt/xfs-dax/BiscuitOS.bin > /dev/null 2>&1

# Running program
BiscuitOS-PAGING-PF-FILE-XFS-DAX-default
