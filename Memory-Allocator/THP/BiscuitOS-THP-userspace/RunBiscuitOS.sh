#!/bin/ash
# 
# Remount
# umount /tmp/
# mount -t tmpfs -o huge=always nodev /tmp/
# Enable shmem huge
echo always > /sys/kernel/mm/transparent_hugepage/shmem_enabled
# Setup scan sleep
echo 3000 > /sys/kernel/mm/transparent_hugepage/khugepaged/scan_sleep_millisecs
# Running program
BiscuitOS-THP-userspace-default &
