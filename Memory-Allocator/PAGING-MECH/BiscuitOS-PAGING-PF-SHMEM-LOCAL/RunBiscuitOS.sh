#!/bin/ash
# 
# Prepare
mkdir -p /mnt/BiscuitOS-LOCAL-SHMEM/
mount -t tmpfs nodev /mnt/BiscuitOS-LOCAL-SHMEM/

dd bs=1M count=2 if=/dev/zero of=/mnt/BiscuitOS-LOCAL-SHMEM/BiscuitOS.mem > /dev/null 2>&1

# Running program
BiscuitOS-PAGING-PF-SHMEM-LOCAL-default-SERVER &
sleep 0.5
BiscuitOS-PAGING-PF-SHMEM-LOCAL-default-CLIENT &
sleep 0.5
echo ""
echo "free"
free
