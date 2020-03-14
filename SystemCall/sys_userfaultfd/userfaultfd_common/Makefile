#
# syscall: sys_userfaultfd
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
TARGET			:= sys_userfaultfd
endif

## Source Code
SRC			+= main.c

## CFlags
LCFLAGS			+= -DCONFIG_BISCUITOS_APP
## Header
LCFLAGS			+= -I./

## X86/X64 Architecture
ifeq ($(ARCH), i386)
CROSS_COMPILE	:=
LCFLAGS		+= -m32
else ifeq ($(ARCH), x86_64)
CROSS_COMPILE	:=
endif

# Compile
B_AS		= $(CROSS_COMPILE)as
B_LD		= $(CROSS_COMPILE)ld
B_CC		= $(CROSS_COMPILE)gcc
B_CPP		= $(CC) -E
B_AR		= $(CROSS_COMPILE)ar
B_NM		= $(CROSS_COMPILE)nm
B_STRIP		= $(CROSS_COMPILE)strip
B_OBJCOPY	= $(CROSS_COMPILE)objcopy
B_OBJDUMP	= $(CROSS_COMPILE)objdump

## Install PATH
ifeq ("$(origin INSPATH)", "command line")
INSTALL_PATH		:= $(INSPATH)
else
INSTALL_PATH		:= ./
endif

all:
	$(B_CC) $(LCFLAGS) -o $(TARGET) $(SRC)

install:
	@cp -rfa $(TARGET) $(INSTALL_PATH)

clean:
	@rm -rf $(TARGET) *.o
