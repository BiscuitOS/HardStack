#!/bin/ash

BiscuitOS-MCE-software-offline-poison-page-default &
sleep 0.2

# Obtain Poison page address
POISON_PADDR=$(cat /tmp/.posion_paddr.txt)
# Software offline poison page
[ ! -f /sys/devices/system/memory/soft_offline_page ] && echo "Pls Enable CONFIG_MEMORY_FAILURE" && exit 1

# Just for debug
# echo 1 > /proc/sys/BiscuitOS/bs_debug_enable
echo ${POISON_PADDR} > /sys/devices/system/memory/soft_offline_page
# echo 0 > /proc/sys/BiscuitOS/bs_debug_enable
echo "Offline Poison Page ${POISON_PADDR}"
