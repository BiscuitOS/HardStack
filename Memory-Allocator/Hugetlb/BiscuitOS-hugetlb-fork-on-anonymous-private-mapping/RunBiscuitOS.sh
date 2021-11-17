#!/bin/ash
# Child and Parent need 2 hugepages
echo 10 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
BiscuitOS-hugetlb-fork-on-anonymous-private-mapping-default
