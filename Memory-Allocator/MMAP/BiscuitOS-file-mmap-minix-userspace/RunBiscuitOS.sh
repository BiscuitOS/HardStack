#!/bin/ash

# Running on Host
if [ $1X = "hostX" ]; then
  dd bs=1M count=1 if=/dev/zero of=disk.img > /dev/null 2>&1
  LOOPDEV=$(sudo losetup -f)
  sudo losetup ${LOOPDEV} disk.img
  sudo mkfs.minix -1 ${LOOPDEV}
  sudo losetup -d ${LOOPDEV} > /dev/null 2>&1
  exit 0
fi

# Running on Guest
# Mount minix
mkdir -p /BiscuitOS-minix/
mount -t minix /usr/bin/disk.img /BiscuitOS-minix/
cd /BiscuitOS-minix/
echo "Mount information                                     [YES]"
mount | grep BiscuitOS
echo ""

echo "Invoke Process                                        [YES]"
BiscuitOS-file-mmap-minix-userspace-default

# hexdump
if [ -f /usr/bin/hexdump ]; then
  echo ""
  echo "Dump Context for /BiscuitOS-minix/BiscuitOS            [YES]"
  hexdump /BiscuitOS-minix/BiscuitOS
fi
