#!/bin/ash

echo "BiscuitOS" > /BiscuitOS
BiscuitOS-file-share-mmap-userspace-default
# dump
echo ""
echo "Dump Context for /BiscuitOS"
hexdump /BiscuitOS
