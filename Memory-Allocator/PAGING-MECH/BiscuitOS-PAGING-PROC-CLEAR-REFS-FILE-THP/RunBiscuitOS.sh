#!/bin/ash
# 
# Running program
echo "Hello BiscuitOS" > /mnt/huge-tmpfs/BiscuitOS.txt

BiscuitOS-PAGING-PROC-CLEAR-REFS-FILE-THP-default &
sleep 0.2
PID=$(pidof "BiscuitOS-PAGING-PROC-CLEAR-REFS-FILE-THP-default")

if [ ! -f "/proc/$PID/smaps" ]; then
    echo "Error: /proc/$PID/smaps does not exist."
    exit 2
fi

TOTAL=0
ACCESS=0

echo "cat /proc/${PID}/smaps Referenced"
while [ 1 ]
do
    REF=`cat /proc/${PID}/smaps | grep "6000000000" -A 24 | grep Referenced`
    REF_NUM=$(echo "$REF" | grep -o '[0-9]\+')
    TOTAL=$((TOTAL + 1))
    [ $REF_NUM == 2048 ] && ACCESS=$((ACCESS + 1))
    PER=$(echo "scale=2; $ACCESS / $TOTAL" | bc)
    PER=$(echo "scale=2; $PER * 100" | bc)
    echo "$REF  Populatirty: $PER%"
    echo 3 > /proc/${PID}/clear_refs # Only effect File-Mapped
    sleep 1
done
