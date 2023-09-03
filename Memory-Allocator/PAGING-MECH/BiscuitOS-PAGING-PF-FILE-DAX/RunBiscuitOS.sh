#!/bin/ash
# 
mkdir -p /mnt/DAX
mkfs.ext4 -b 4096 -E stride=512 -F /dev/pmem0
mount -o dax=always,lazytime /dev/pmem0 /mnt/DAX/
echo always > /sys/kernel/mm/transparent_hugepage/enabled
dd bs=1M count=4 if=/dev/zero of=/mnt/DAX/BiscuitOS.txt > /dev/null 2>&1
truncate -s 2M /mnt/DAX/BiscuitOS.txt

echo ""
echo "Running Programs"
# Running program
BiscuitOS-PAGING-PF-FILE-DAX-default
