#!/bin/ash

# Running on Host
if [ $1X = "hostX" ]; then
  dd bs=1M count=1 if=/dev/zero of=disk.img > /dev/null 2>&1
  LOOPDEV=$(sudo losetup -f)
  sudo losetup ${LOOPDEV} disk.img
  sudo mkfs.msdos ${LOOPDEV}
  sudo losetup -d ${LOOPDEV} > /dev/null 2>&1
  exit 0
fi

# Running on Guest
# Mount msdos
mkdir -p /BiscuitOS-msdos/
mount -t msdos /usr/bin/disk.img /BiscuitOS-msdos/
cd /BiscuitOS-msdos/
echo "Mount information                                     [YES]"
mount | grep BiscuitOS
echo ""

echo "Invoke Process                                        [YES]"
BiscuitOS-file-mmap-msdos-userspace-default

# hexdump
if [ -f /usr/bin/hexdump ]; then
  echo ""
  echo "Dump Context for /BiscuitOS-msdos/BiscuitOS           [YES]"
  hexdump /BiscuitOS-msdos/BiscuitOS
fi
