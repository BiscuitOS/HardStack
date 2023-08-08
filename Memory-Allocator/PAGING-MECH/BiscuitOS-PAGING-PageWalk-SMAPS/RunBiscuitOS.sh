#!/bin/ash

APP &
sleep 2

PID=$(pidof "APP")
echo "cat /proc/${PID}/smaps"
echo ""
cat /proc/${PID}/smaps | grep "/dev/zero" -A 24
