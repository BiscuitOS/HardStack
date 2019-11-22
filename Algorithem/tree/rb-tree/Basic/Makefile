#
# RB-Tree
#
# (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
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
CFLAGS += -I./

# SRC
SRC += rb_run.c rbtree.c

# Target
ifeq ($(TARGETA), )
TARGET=rbtree_demo
else
TARGET=$(TARGETA)
endif

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

install:
	@cp -rfa $(TARGET) $(INSTALL_PATH)

clean:
	@rm -rf *.ko *.o *.mod.o *.mod.c *.symvers *.order \
               .*.o.cmd .tmp_versions *.ko.cmd .*.ko.cmd $(TARGET)
