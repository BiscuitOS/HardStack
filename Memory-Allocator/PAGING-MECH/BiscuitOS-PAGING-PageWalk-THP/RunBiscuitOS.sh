#!/bin/ash

echo always > /sys/kernel/mm/transparent_hugepage/enabled

cat /proc/meminfo | grep Huge
echo "cat /proc/meminfo | grep Huge"
echo ""

APP &

sleep 0.1
cat /proc/meminfo | grep Huge
echo "cat /proc/meminfo | grep Huge"
echo ""
