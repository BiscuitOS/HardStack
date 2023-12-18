#!/bin/ash

APP &
sleep 0.1

# MAPS
PID=$(pidof "APP")
echo ""
echo ""
echo "MAP: /proc/${PID}/maps"
cat /proc/${PID}/maps
