#!/bin/ash
# 
# Running program
BiscuitOS-PAGING-PROC-CLEAR-REFS-ANON-default &
sleep 0.2
PID=$(pidof "BiscuitOS-PAGING-PROC-CLEAR-REFS-ANON-default")

if [ ! -f "/proc/$PID/smaps" ]; then
    echo "Error: /proc/$PID/smaps does not exist."
    exit 2
fi

echo "cat /proc/${PID}/smaps Referenced"
while [ 1 ]
do
    cat /proc/${PID}/smaps | grep "6000000000" -A 24 | grep Referenced
    echo 2 > /proc/${PID}/clear_refs # Only affects anonymous pages
    sleep 1
done
