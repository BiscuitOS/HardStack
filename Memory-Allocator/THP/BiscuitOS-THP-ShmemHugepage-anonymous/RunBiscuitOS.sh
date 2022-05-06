#!/bin/ash
# 
# Remount
# umount /tmp/
# mount -t tmpfs -o huge=always nodev /tmp/
# Enable shmem huge
echo always > /sys/kernel/mm/transparent_hugepage/shmem_enabled
# Running program
BiscuitOS-THP-ShmemHugepage-anonymous-default &
