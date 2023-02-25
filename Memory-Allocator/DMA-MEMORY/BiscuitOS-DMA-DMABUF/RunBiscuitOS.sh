#/bin/ash
insmod /lib/modules/$(uname -r)/extra/DMABUF-export.ko
insmod /lib/modules/$(uname -r)/extra/DMABUF-import.ko
export-app &
sleep 0.1
import-app0
import-app1
