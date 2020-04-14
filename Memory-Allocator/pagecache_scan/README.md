# pgcache_scan by writen
pagecache scan for inode in super_block
pagecache scan for inode in process open file inode

use:

    make

    make load

    dmesg

    make unload

advanced use:    

  process open file's inode:
  
    echo 0 > /proc/sys/vm/pgcache_scan/pgcache_scan_mode

       or
  inode on supper_block:
  
    echo 1 > /proc/sys/vm/pgcache_scan/pgcache_scan_mode

       or
  open file's inode and inode on supper_block:
  
    echo 2 > /proc/sys/vm/pgcache_scan/pgcache_scan_mode

  notice: 0 is default

  print top n list to dmesg:
  
    echo 10 > /proc/sys/vm/pgcache_scan/pgcache_scan_top_n
       or
    echo 20 > /proc/sys/vm/pgcache_scan/pgcache_scan_top_n
       or
    echo 50 > /proc/sys/vm/pgcache_scan/pgcache_scan_top_n
    ......................
    dmesg

  file inode is deleted but being used:
  
    echo 1 > /proc/sys/vm/pgcache_scan/pgcache_scan_file_deleted_but_used

    echo 10 > /proc/sys/vm/pgcache_scan/pgcache_scan_top_n

    dmesg

notice:

   valid on CentOS Linux release 7.3.1611 

uname -a:

   Linux localhost 3.10.0-514.26.2.el7.x86_64


