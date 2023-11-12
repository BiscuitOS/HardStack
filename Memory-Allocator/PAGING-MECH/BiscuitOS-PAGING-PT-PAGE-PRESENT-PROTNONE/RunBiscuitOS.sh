#!/bin/ash
# 
# NUMA Balancing Setup
#  sysctl_numa_balancing_scan_period_min=0
#  sysctl_numa_balancing_scan_period_max=0
#  sysctl_numa_balancing_scan_size=256
#  sysctl_numa_balancing_scan_delay=0
# Running program
echo 1 > /proc/sys/kernel/numa_balancing
BiscuitOS-PAGING-PT-PAGE-PRESENT-PROTNONE-default &
