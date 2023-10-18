#!/bin/ash
# 
# EXT4 with DAX
mkdir -p /mnt/ext4-DAX
#mkfs.ext4 -b 4096 -E stride=512 -F /dev/pmem0
mkfs.ext4 -b 4096 -E stride=512 -F /dev/pmem0
mount -t ext4 -o dax=always,lazytime /dev/pmem0 /mnt/ext4-DAX/
echo always > /sys/kernel/mm/transparent_hugepage/enabled
dd bs=1M count=8 if=/dev/zero of=/mnt/ext4-DAX/BiscuitOS.txt > /dev/null 2>&1
#dd bs=1M count=2 seek=0 if=/dev/zero of=/dev/pmem0 > /dev/null 2>&1
#rm -rf /tmp/tmp.file > /dev/null 2>&1
#truncate -s 4M /mnt/DAX/BiscuitOS.txt

# Running program
BiscuitOS-PAGING-HUGE-PF-EXT4-HUGE-DAX-default &
sleep 0.1
