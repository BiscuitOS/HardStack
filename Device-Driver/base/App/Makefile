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
SRC			:= $(wildcard $(PWD)/src/*.c)
SRC			+= main.c

## CFlags
LCFLAGS			+= -DCONFIG_BISCUITOS_APP
## Header
LCFLAGS			+= -I./ -I$(PWD)/include

## X86/X64 Architecture
ifeq ($(ARCH), i386)
CROSS_COMPILE   :=
else ifeq ($(ARCH), x64)
CROSS_COMPILE   :=
endif

# Compile
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

## Install PATH
ifeq ("$(origin INSPATH)", "command line")
INSTALL_PATH		:= $(INSPATH)
else
INSTALL_PATH		:= ./
endif

all:
	$(CC) $(LCFLAGS) -o $(TARGET) $(SRC)

install:
	@cp -rfa $(TARGET) $(INSTALL_PATH)

clean:
	@rm -rf $(TARGET) *.o
