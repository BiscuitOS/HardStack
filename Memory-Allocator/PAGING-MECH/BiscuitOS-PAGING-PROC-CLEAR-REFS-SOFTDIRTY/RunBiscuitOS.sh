#!/bin/ash
# 
# Running program
BiscuitOS-PAGING-PROC-CLEAR-REFS-SOFTDIRTY-default &
sleep 0.2
PID=$(pidof "BiscuitOS-PAGING-PROC-CLEAR-REFS-SOFTDIRTY-default")

if [ ! -f "/proc/$PID/pagemap" ]; then
    echo "Error: /proc/$PID/pagemap does not exist."
    exit 2
fi

echo "cat /proc/${PID}/pagemap SoftDirty"
while [ 1 ]
do
    SOFTDIRTY ${PID} 0x6000000000
    echo 4 > /proc/${PID}/clear_refs
    sleep 1
done
