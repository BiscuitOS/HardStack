#!/bin/ash

# Running on Host
if [ $1X = "hostX" ]; then
  dd bs=1M count=2 if=/dev/zero of=disk.img > /dev/null 2>&1
  LOOPDEV=$(sudo losetup -f)
  sudo losetup ${LOOPDEV} disk.img
  sudo mkfs.ext3 ${LOOPDEV}
  sudo losetup -d ${LOOPDEV} > /dev/null 2>&1
  exit 0
fi

# Running on Guest
# Mount ext3
mkdir -p /BiscuitOS-ext3/
mount -t ext3 /usr/bin/disk.img /BiscuitOS-ext3/
cd /BiscuitOS-ext3/
echo "Mount information                                     [YES]"
mount | grep BiscuitOS
echo ""

echo "Invoke Process                                        [YES]"
BiscuitOS-file-mmap-ext3-userspace-default

# hexdump
if [ -f /usr/bin/hexdump ]; then
  echo ""
  echo "Dump Context for /BiscuitOS-ext3/BiscuitOS            [YES]"
  hexdump /BiscuitOS-ext3/BiscuitOS
fi
