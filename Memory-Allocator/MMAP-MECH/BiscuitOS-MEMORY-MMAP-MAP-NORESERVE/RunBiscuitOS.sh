#!/bin/ash
# 
# Allocate 10 Hugepage to default hugepage pool
echo 10 > /proc/sys/vm/nr_hugepages
# Running program
BiscuitOS-MEMORY-MMAP-MAP-NORESERVE-default
