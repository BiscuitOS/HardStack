#!/bin/ash
# 
# Prepare
echo "Bello BiscuitOS" > /dev/shm/BiscuitOS.mem
# Running program
BiscuitOS-PAGING-PF-SHMEM-POSIX-default-SERVER &
sleep 0.5
BiscuitOS-PAGING-PF-SHMEM-POSIX-default-CLIENT &
