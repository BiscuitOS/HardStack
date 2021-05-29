#
# Application Project
#
# (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

## Target
ifeq ("$(origin TARGETA)", "command line")
TARGET			:= $(TARGETA)
else
TARGET			:= BiscuitOS-address-space-data-segment-global-default
endif

## Source Code
SRC_0			:= main-A.c
SRC_1			:= main-B.c
LIB_0			:= BiscuitOS-lib.c

LIB_EPATH		:= $(shell cd ../../../rootfs/rootfs/lib; pwd)
## CFlags
LCFLAGS			+= -I./ -I$(PWD)/include -L$(LIB_EPATH)
LCFLAGS			+= -no-pie -fno-pic # -fpic

DOT			:= -
## X86/X64 Architecture
ifeq ($(ARCH), i386)
CROSS_COMPILE	=
LCFLAGS		+= -m32
DOT		:=
else ifeq ($(ARCH), x86_64)
CROSS_COMPILE   :=
DOT		:=
endif

# Compile
B_AS		= $(CROSS_COMPILE)$(DOT)as
B_LD		= $(CROSS_COMPILE)$(DOT)ld
B_CC		= $(CROSS_COMPILE)$(DOT)gcc
B_CPP		= $(CC) -E
B_AR		= $(CROSS_COMPILE)$(DOT)ar
B_NM		= $(CROSS_COMPILE)$(DOT)nm
B_STRIP		= $(CROSS_COMPILE)$(DOT)strip
B_OBJCOPY	= $(CROSS_COMPILE)$(DOT)objcopy
B_OBJDUMP	= $(CROSS_COMPILE)$(DOT)objdump
B_READELF	= $(CROSS_COMPILE)$(DOT)readelf

## Install PATH
ifeq ("$(origin INSPATH)", "command line")
INSTALL_PATH		:= $(INSPATH)
LIB_PATH		:= $(subst usr/bin,lib, $(INSPATH))
else
INSTALL_PATH		:= ./
endif

all:
	$(B_CC)	$(LCFLAGS) -E $(SRC_0) -o $(TARGET)-0.i
	$(B_CC)	$(LCFLAGS) -E $(SRC_1) -o $(TARGET)-1.i
	$(B_CC)	$(LCFLAGS) -E $(LIB_0) -o $(TARGET)-2.i
	$(B_CC) $(LCFLAGS) -S $(TARGET)-0.i -o $(TARGET)-0.s
	$(B_CC) $(LCFLAGS) -S $(TARGET)-1.i -o $(TARGET)-1.s
	$(B_CC) $(LCFLAGS) -S $(TARGET)-2.i -o $(TARGET)-2.s
	$(B_CC) $(LCFLAGS) -c $(TARGET)-0.s -o $(TARGET)-0.o
	$(B_CC) $(LCFLAGS) -c $(TARGET)-1.s -o $(TARGET)-1.o
	$(B_CC) -shared $(LCFLAGS) -c $(TARGET)-2.s -o lib$(TARGET)-lib.so
	@sudo cp -rfa lib$(TARGET)-lib.so $(LIB_EPATH)
	$(B_CC) $(LCFLAGS) -o $(TARGET)-A $(TARGET)-0.o -l $(TARGET)-lib
	$(B_CC) $(LCFLAGS) -o $(TARGET)-B $(TARGET)-1.o -l $(TARGET)-lib
	$(B_OBJDUMP) -sSdhx $(TARGET)-0.o > $(TARGET)-0.bs
	$(B_OBJDUMP) -sSdhx $(TARGET)-1.o > $(TARGET)-1.bs
	$(B_OBJDUMP) -sSdhx lib$(TARGET)-lib.so > $(TARGET)-lib.bs
	$(B_OBJDUMP) -sSdhx $(TARGET)-A > $(TARGET)-A.bs
	$(B_OBJDUMP) -sSdhx $(TARGET)-B > $(TARGET)-B.bs
	$(B_READELF) -a $(TARGET)-A > $(TARGET)-A.ebs
	$(B_READELF) -a $(TARGET)-B > $(TARGET)-B.ebs

install:
	@cp -rfa $(TARGET)-A $(TARGET)-B $(INSTALL_PATH)

clean:
	@rm -rf $(TARGET)-A $(TARGET)-B *.o *.i *.s *.bs *.ebs *.so
	@sudo rm -rf $(LIB_EPATH)/lib$(TARGET)-lib.so
