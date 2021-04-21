#!/bin/ash

# Running on Host
if [ $1X = "hostX" ]; then
  dd bs=1M count=2 if=/dev/zero of=disk.img > /dev/null 2>&1
  LOOPDEV=$(sudo losetup -f)
  sudo losetup ${LOOPDEV} disk.img
  sudo mkfs.ext4 ${LOOPDEV}
  sudo losetup -d ${LOOPDEV} > /dev/null 2>&1
  exit 0
fi

# Running on Guest
# Mount ext4
mkdir -p /BiscuitOS-ext4/
mount -t ext4 /usr/bin/disk.img /BiscuitOS-ext4/
cd /BiscuitOS-ext4/
echo "Mount information                                     [YES]"
mount | grep BiscuitOS
echo ""

echo "Invoke Process                                        [YES]"
BiscuitOS-file-mmap-ext4-userspace-default

# hexdump
if [ -f /usr/bin/hexdump ]; then
  echo ""
  echo "Dump Context for /BiscuitOS-ext4/BiscuitOS            [YES]"
  hexdump /BiscuitOS-ext4/BiscuitOS
fi
