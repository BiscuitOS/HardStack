#!/bin/ash

APP &
sleep 2

PID=$(pidof "APP")
echo "cat /proc/${PID}/numa_maps"
echo ""
cat /proc/${PID}/numa_maps | grep "/dev/zero" -A 2 -B 2
