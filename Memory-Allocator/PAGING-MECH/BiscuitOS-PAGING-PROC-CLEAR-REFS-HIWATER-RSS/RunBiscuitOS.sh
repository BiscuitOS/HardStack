#!/bin/ash
# 
# Running program
BiscuitOS-PAGING-PROC-CLEAR-REFS-HIWATER-RSS-default &
sleep 0.2
PID=$(pidof "BiscuitOS-PAGING-PROC-CLEAR-REFS-HIWATER-RSS-default")

if [ ! -f "/proc/$PID/clear_refs" ]; then
    echo "Error: /proc/$PID/clear_refs does not exist."
    exit 2
fi

echo "cat /proc/${PID}/clear_refs HIWATER_RSS"
while [ 1 ]
do
    VMHWM=`cat /proc/${PID}/status | grep VmHWM`
    RSS=`cat /proc/${PID}/smaps | grep "6000000000" -A 24 | grep Rss`
    echo "PID-${PID}: ${VMHWM}   ${RSS}"
    echo 5 > /proc/${PID}/clear_refs
    sleep 1
done
