#!/bin/ash

# Running on Host
if [ $1X = "hostX" ]; then
  dd bs=1M count=1 if=/dev/zero of=disk.img > /dev/null 2>&1
  LOOPDEV=$(sudo losetup -f)
  sudo losetup ${LOOPDEV} disk.img
  sudo mkfs.vfat ${LOOPDEV}
  sudo losetup -d ${LOOPDEV} > /dev/null 2>&1
  exit 0
fi

# Running on Guest
# Mount vfat
mkdir -p /BiscuitOS-vfat/
mount -t vfat /usr/bin/disk.img /BiscuitOS-vfat/
cd /BiscuitOS-vfat/
echo "Mount information                                     [YES]"
mount | grep BiscuitOS
echo ""

echo "Invoke Process                                        [YES]"
BiscuitOS-file-mmap-vfat-userspace-default

# hexdump
if [ -f /usr/bin/hexdump ]; then
  echo ""
  echo "Dump Context for /BiscuitOS-vfat/BiscuitOS             [YES]"
  hexdump /BiscuitOS-vfat/BiscuitOS
fi
