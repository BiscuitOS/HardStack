#
# pgcache_scan
#
# (C) .....
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

ifneq ($(KERNELRELEASE),)

## Target
ifeq ("$(origin MODULE_NAME)", "command line")
MODULE_NAME		:= $(MODULE_NAME)
else
MODULE_NAME		:= pgcache_scan
endif
obj-m			:= $(MODULE_NAME).o

## Source Code
$(MODULE_NAME)-m	+= kernel.o sysctl.o cache_scan.o

## CFLAGS
ccflags-y		+= -Wall -g
## EXTRA_CFLAGS
ccflags-y		+= -g -Wall -Wmissing-prototypes -Wstrict-prototypes
## ADDR
ADDR=$(shell grep -w  kallsyms_lookup_name /proc/kallsyms)

else

## Parse argument
## Default support ARM32
ifeq ("$(origin BSROOT)", "command line")
BSROOT=$(BSROOT)
KERNELDIR=$(BSROOT)/linux/linux
else
BSROOT=$(shell pwd)
KERNELDIR=/lib/modules/$(shell uname -r)/build
endif

ifeq ("$(origin ARCH)", "command line")
ARCH=$(ARCH)
else
ARCH=x86_64
endif

ifeq ("$(origin CROSS_TOOLS)", "command line")
CROSS_TOOLS=$(CROSS_TOOLS)
else
CROSS_TOOLS=
endif

## Don't Edit
# CROSS PATH
CROSS_COMPILE_PATH=$(BSROOT)/$(CROSS_TOOLS)/$(CROSS_TOOLS)/bin
CROSS_COMPILE=$(CROSS_COMPILE_PATH)/$(CROSS_TOOLS)-
# HEADER PATH
INCLUDEDIR=$(KERNELDIR)/include
# ARCHITECTURE PATH
ARCHINCLUDEDIR=$(KERNELDIR)/arch/$(ARCH)/include
# ROOTFS PATH
INSTALLDIR=$(BSROOT)/rootfs/rootfs/

## X86/X64 Architecture
ifeq ($(ARCH), i386)
CROSS_COMPILE	:=
else ifeq ($(ARCH), x86_64)
CROSS_COMPILE	:=
endif

## Compile
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

# FLAGS
CFLAGS += -I$(INCLUDEDIR) -I$(ARCHINCLUDEDIR)

all: $(OBJS)
	make -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) \
                CROSS_COMPILE=$(CROSS_COMPILE) modules

load:
	@for i in $(ADDR); \
	do \
		insmod $(MODULE_NAME).ko kallsyms_lookup_name_addr=0x$$i; \
		echo insmod $(MODULE_NAME).ko; \
		break; \
	done

unload:
	rmmod ${MODULE_NAME}

reload:
	make unload
	make load

status:
	@lsmod |grep ${MODULE_NAME}

install:
	make -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) \
		INSTALL_MOD_PATH=$(INSTALLDIR) modules_install
	@if [ -f RunBiscuitOS_pgcache_scan.sh ]; then \
		chmod 755 RunBiscuitOS_pgcache_scan.sh ; \
		cp -rfa RunBiscuitOS_pgcache_scan.sh $(BSROOT)/rootfs/rootfs/usr/bin ; \
	fi
		

clean:
	@rm -rf *.ko *.o *.mod.o *.mod.c *.symvers *.order \
               .*.o.cmd .tmp_versions *.ko.cmd .*.ko.cmd *o.ur-safe

endif
