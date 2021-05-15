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
TARGET			:= BiscuitOS_app
endif

## Source Code
SRC_0			:= main.c
SRC_1			:= BiscuitOS.c

## CFlags
LCFLAGS			+= -DCONFIG_BISCUITOS_APP
## Header
LCFLAGS			+= -I./ -I$(PWD)/include

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
else
INSTALL_PATH		:= ./
endif

all:
	$(B_CC)	$(LCFLAGS) -E $(SRC_0) -o $(TARGET)_0.i
	$(B_CC)	$(LCFLAGS) -E $(SRC_1) -o $(TARGET)_1.i
	$(B_CC) $(LCFLAGS) -S $(TARGET)_0.i -o $(TARGET)_0.s
	$(B_CC) $(LCFLAGS) -S $(TARGET)_1.i -o $(TARGET)_1.s
	$(B_CC) $(LCFLAGS) -c $(TARGET)_0.s -o $(TARGET)_0.o
	$(B_CC) $(LCFLAGS) -c $(TARGET)_1.s -o $(TARGET)_1.o
	$(B_CC) -static $(LCFLAGS) -o $(TARGET) $(TARGET)_0.o $(TARGET)_1.o
	$(B_OBJDUMP) -sSdhx $(TARGET) > $(TARGET).bs
	$(B_READELF) -l $(TARGET) > $(TARGET).ebs

install:
	@cp -rfa $(TARGET) $(INSTALL_PATH)

clean:
	@rm -rf $(TARGET) *.o *.i *.s *.bs *.ebs
