#!/bin/ash
# 

# Open PORT
# nc -lp 8890 &

# Mount /dev/shm
mount | grep "/dev/shm" > /dev/null
[ $? -ne 0 ] && mkdir /dev/shm && mount -t hugetlbfs none /dev/shm -o pagesize=2048K

echo 10 > /proc/sys/vm/nr_hugepages
BiscuitOS-hugetlb-anonymous-share-mapping-sysv-default-Server &
sleep 1
BiscuitOS-hugetlb-anonymous-share-mapping-sysv-default-Client &
sleep 2

