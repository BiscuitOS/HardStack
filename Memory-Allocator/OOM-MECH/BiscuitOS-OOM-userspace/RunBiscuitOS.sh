#!/bin/ash

# Build cgroup
# Must enable macro: CONFIG_MEMCG
mount -t cgroup -o memory memory /sys/fs/cgroup
[ $? -ne 0 ] && echo "Enable Kernel macro: CONFIG_MEMCG!" && exit 0
mkdir -p /sys/fs/cgroup/memory/BiscuitOS

# Run App
BiscuitOS-OOM-userspace-default &
sleep 0.2
pid=$(pidof BiscuitOS-OOM-userspace-default)

# Suetup Cgroup
# Add process and limit max memory
echo $pid > /sys/fs/cgroup/memory/BiscuitOS/cgroup.procs
echo 512K > /sys/fs/cgroup/memory/BiscuitOS/memory.limit_in_bytes

# Forbid OOM killer
# echo 1 > /sys/fs/cgroup/memory/BiscuitOS/memory.oom_control

echo "***** BiscuitOS Trigger OOM *****"
echo ""
