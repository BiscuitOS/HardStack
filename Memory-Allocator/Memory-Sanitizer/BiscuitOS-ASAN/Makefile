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
SRC			+= main.c

## CFlags
LCFLAGS			+= -DCONFIG_BISCUITOS_APP
## Header
LCFLAGS			+= -I./ -I$(PWD)/include
## ASAN Flags
ASANCFLAGS		:= -fsanitize=leak
ASANCFLAGS		+= -fsanitize=address
ASANCFLAGS		+= -fsanitize=memory
ASANCFLAGS		+= -fno-omit-frame-pointer
ASANCFLAGS		+= -fno-optimize-sibling-calls -O1
ASANCFLAGS		+= -g

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

## Install PATH
ifeq ("$(origin INSPATH)", "command line")
INSTALL_PATH		:= $(INSPATH)
else
INSTALL_PATH		:= ./
endif

all:
	# Preprocessing
	$(B_CC) $(LCFLAGS) -E $(SRC) -o $(TARGET)-Prep.i
	# Compilation
	$(B_CC) -S $(TARGET)-Prep.i -o $(TARGET)-ASM.s
	# Assemble
	$(B_CC) -C $(TARGET)-ASM.s -o $(TARGET)-obj.o
	# Linking
	$(B_CC) $(TARGET)-obj.o -shared -o $(TARGET)-NOASAN
	# Disassembling object file
	$(B_OBJDUMP) -sSdhx $(TARGET)-obj.o > $(TARGET)-Object.obs
	# Target
	$(B_CC) $(LCFLAGS) $(ASANCFLAGS) -o $(TARGET) $(SRC)
	# Disassembling target file
	$(B_OBJDUMP) -sSdhx $(TARGET) > $(TARGET)-target.obs
	# Disassembling ASAN target file
	$(B_OBJDUMP) -sSdhx $(TARGET)-NOASAN > $(TARGET)-target-noasan.obs

install:
	@cp -rfa $(TARGET) $(INSTALL_PATH)
	@cp -rfa $(TARGET)-NOASAN $(INSTALL_PATH)

clean:
	@rm -rf $(TARGET) *.o *.i *.s *.obs $(TARGET)-NOASAN
