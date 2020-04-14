#!/bin/ash

# Running pgcache_scan
#
# (C) ....
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

ADDR=$(shell grep -w  kallsyms_lookup_name /proc/kallsyms)
MODULE_PATH=/lib/modules/$(uname -r)/extra
KEYNAME=pgcache_scan
MODULE=${KEYNAME}-0.0.1.ko

OPT=$1
ARGS=$2

load()
{
	for i in ${ADDR}
	do
		insmod ${MODULE_PATH}/${MODULE} \
					kallsyms_lookup_name_addr=0x$$i;
		echo insmod ${MODULE}
		break;
	done
}

unload()
{
	rmmod ${MODULE}
}

reload()
{
	unload
	load
}

status()
{
	lsmod | grep ${KEYNAME}
}

case $OPT in
	"load")
		load
	;;
	"unload")
		unload
	;;
	"reload")
		reload
	;;
	"status")
		status
	;;
esac
