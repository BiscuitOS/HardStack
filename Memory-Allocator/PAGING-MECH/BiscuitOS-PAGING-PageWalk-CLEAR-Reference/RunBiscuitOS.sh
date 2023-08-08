#!/bin/ash

APP &
sleep 1

PID=$(pidof "APP")
echo "cat /proc/${PID}/smaps"
echo ""
while [ 1 ]
do
	sleep 1
	cat /proc/${PID}/smaps | grep "/dev/zero" -A 12 | grep Referenced
	echo 1 > /proc/${PID}/clear_refs
done
