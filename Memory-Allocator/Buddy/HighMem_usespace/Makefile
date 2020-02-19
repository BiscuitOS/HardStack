#
# Buddy Memory Allocator
#
# (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

# Install PATH
ifeq ($(INSPATH), )
INSTALL_PATH=./
else
INSTALL_PATH=$(INSPATH)
endif

# CROSS_COMPILE form argument

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

# FLAGS
LCFLAGS += -I./ -I$(PWD)/include

# SRC
SRC := $(wildcard $(PWD)/mm/*.c)
SRC += main.c

# Configuration
CONFIG += -DCONFIG_MEMORY_SIZE=0x1000000
CONFIG += -DCONFIG_PHYS_BASE=0x60000000
CONFIG += -DCONFIG_HIGHMEM
CONFIG += -DCONFIG_HIGHMEM_SIZE=0x200000

# Target
ifeq ($(TARGETA), )
TARGET=biscuitos
LCFLAGS += -m32
else
TARGET=$(TARGETA)
endif

all:
	@$(CC) $(LCFLAGS) $(CONFIG) -o $(TARGET) $(SRC)

install:
	@cp -rfa $(TARGET) $(INSTALL_PATH)

clean:
	@rm -rf *.ko *.o *.mod.o *.mod.c *.symvers *.order \
               .*.o.cmd .tmp_versions *.ko.cmd .*.ko.cmd $(TARGET)
