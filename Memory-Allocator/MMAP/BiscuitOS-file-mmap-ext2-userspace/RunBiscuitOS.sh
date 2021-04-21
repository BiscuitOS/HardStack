#!/bin/ash

# Running on Host
if [ $1X = "hostX" ]; then
  dd bs=1M count=1 if=/dev/zero of=disk.img > /dev/null 2>&1
  LOOPDEV=$(sudo losetup -f)
  sudo losetup ${LOOPDEV} disk.img
  sudo mkfs.ext2 ${LOOPDEV}
  sudo losetup -d ${LOOPDEV} > /dev/null 2>&1
  exit 0
fi

# Running on Guest
# Mount ext2
mkdir -p /BiscuitOS-ext2/
mount -t ext2 /usr/bin/disk.img /BiscuitOS-ext2/
cd /BiscuitOS-ext2/
echo "Mount information                                     [YES]"
mount | grep BiscuitOS
echo ""

echo "Invoke Process                                        [YES]"
BiscuitOS-file-mmap-ext2-userspace-default

# hexdump
if [ -f /usr/bin/hexdump ]; then
  echo ""
  echo "Dump Context for /BiscuitOS-ext2/BiscuitOS            [YES]"
  hexdump /BiscuitOS-ext2/BiscuitOS
fi
