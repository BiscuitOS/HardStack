#!/bin/ash

# Mount tmpfs
mkdir -p /BiscuitOS-tmpfs/
mount -t tmpfs -o noexec nodev /BiscuitOS-tmpfs/
cd /BiscuitOS-tmpfs/
echo "Mount information                                     [YES]"
mount | grep BiscuitOS
echo ""

echo "Invoke Process                                        [YES]"
BiscuitOS-sys-mmap-example-00-default

# hexdump
if [ -f /usr/bin/hexdump ]; then
  echo ""
  echo "Dump Context for /BiscuitOS-tmpfs/BiscuitOS           [YES]"
  hexdump /BiscuitOS-tmpfs/BiscuitOS
fi
