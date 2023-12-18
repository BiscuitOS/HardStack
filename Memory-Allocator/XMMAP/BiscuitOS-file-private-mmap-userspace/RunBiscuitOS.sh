#!/bin/ash

echo "BiscuitOS" > /BiscuitOS
BiscuitOS-file-private-mmap-userspace-default
# dump
echo ""
echo "Dump Context for /BiscuitOS"
hexdump /BiscuitOS
