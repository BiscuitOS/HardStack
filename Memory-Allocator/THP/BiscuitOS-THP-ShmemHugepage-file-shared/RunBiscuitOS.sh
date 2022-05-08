#!/bin/ash
# 
# Mount huge tmpfs
mkdir -p /BiscuitOS-tmpfs/
mount -t tmpfs nodev -o huge=always /BiscuitOS-tmpfs/
# Running program
BiscuitOS-THP-ShmemHugepage-file-shared-default &
