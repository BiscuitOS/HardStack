SHELL	= /bin/bash

topdir	:= $(shell cd $(src)/.. && pwd)

TARGET_MODULE	:= xdemo

EXTRA_CFLAGS	:= -I$(topdir)/include

ifneq ($(KERNELRELEASE),)
	$(TARGET_MODULE)-objs := pcie.o
	obj-m := $(TARGET_MODULE).o
else
	BUILDSYSTEM_DIR:=/lib/modules/$(shell uname -r)/build
	PWD:=$(shell pwd)
all :
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean

install: all
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules_install
endif

