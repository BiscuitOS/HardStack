#!/bin/ash
# 
#!/bin/ash
# 
# Check EXT3-fs
if ! mount | grep vdc | grep -q ext3; then
   echo "EXT3-FS Don't mount"
   exit
fi

# FAULT AROUND
# echo 12288 > /sys/kernel/debug/fault_around_bytes
# Running program
BiscuitOS-PAGING-PF-FILE-EXT3-FR-default
