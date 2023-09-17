#!/bin/ash
# 
# Modify SWAP Parameters
#  /sys/module/zswap/parameters
# SWAP Information
#  /sys/kernel/debug/zswap
echo always > /sys/kernel/mm/transparent_hugepage/enabled
# Running program
BiscuitOS-PAGING-PF-THP-ANON-ZSWAP-default
