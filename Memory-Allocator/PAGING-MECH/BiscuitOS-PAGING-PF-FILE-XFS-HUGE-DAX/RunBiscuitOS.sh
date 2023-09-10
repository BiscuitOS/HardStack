#!/bin/ash
# 
# XFS with DAX
mkfs.xfs -f -d su=2m,sw=1 -m reflink=0 /dev/pmem0
mkdir -p /mnt/xfs-huge-dax
mount -t xfs -o dax=always /dev/pmem0 /mnt/xfs-huge-dax
xfs_io -c "extsize 2m" /mnt/xfs-huge-dax
dd bs=1M count=4 if=/dev/zero of=/mnt/xfs-huge-dax/BiscuitOS.bin > /dev/null 2>&1

# Running program
BiscuitOS-PAGING-PF-FILE-XFS-HUGE-DAX-default &
sleep 0.1
