#/bin/ash
insmod /lib/modules/$(uname -r)/extra/DMABUF-Export-Capture.ko
insmod /lib/modules/$(uname -r)/extra/DMABUF-Import-GPUA.ko
insmod /lib/modules/$(uname -r)/extra/DMABUF-Import-GPUB.ko
APP-Export-Capture &
sleep 0.1
APP-Import-GPUA
sleep 1
APP-Import-GPUB
