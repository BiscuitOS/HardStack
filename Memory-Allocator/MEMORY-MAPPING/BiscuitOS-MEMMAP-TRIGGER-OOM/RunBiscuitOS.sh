#!/bin/ash
#
# KERNEL: CONFIG_MEMCG=y
#         CONFIG_MEMCG_SWAP=y
#         CONFIG_MEMCG_KMEM=y
# 
# Prepare CGROUP
mkdir -p /mnt/CGROUP
mount -t cgroup -o memory,name=ONDEMAND-MEMORU MEMORY /mnt/CGROUP
mkdir -p /mnt/CGROUP/MEMORY
# Configuration CGROUP
cd /mnt/CGROUP/MEMORY

sleep 0.2
# Running Program
BiscuitOS-MEMMAP-TRIGGER-OOM-default &
PID=$(pgrep BiscuitOS-MEMMAP-TRIGGER-OOM-default)

# Add Program into CGROUP
echo ${PID} > /mnt/CGROUP/MEMORY/cgroup.procs

# Setup MEMORY CGROUP
# 1. Setup max memory
echo 800K > memory.limit_in_bytes
# 2. Forbidden SWAP-OUT
sleep 0.1
echo 0 > memory.swappiness
# 3. Enable OOM
sleep 0.1
echo 1 > memory.oom_control
