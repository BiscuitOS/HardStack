#!/bin/ash
# 
#!/bin/ash
# 
# Check FAT-fs
if ! mount | grep vdg | grep -q vfat; then
   echo "FAT-FS Don't mount"
   exit
fi

# Running program
BiscuitOS-PAGING-PF-FILE-FAT-COW-default
