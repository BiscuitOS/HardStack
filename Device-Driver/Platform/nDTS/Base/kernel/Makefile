# platform_core_module
#
# (C) 2020.09.23 BiscuitOS <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

## Default Setup
ROOT        := /xspace/OpenSource/BiscuitOS/BiscuitOS/output/linux-5.0-i386
BSROOT      := /xspace/OpenSource/BiscuitOS/BiscuitOS
ARCH        := i386
CROSS_NAME  := 
CROSS_PATH  := $(ROOT)/$(CROSS_NAME)/$(CROSS_NAME)
CROSS_TOOL  := $(CROSS_PATH)/bin/$(CROSS_NAME)-
PACK        := $(ROOT)/RunBiscuitOS.sh
DL          := $(BSROOT)/dl
BSCORE      := $(BSROOT)/scripts/package/bsbit_core.sh
BSDEPD      := $(BSROOT)/scripts/package/bsbit_dependence.sh
PACKDIR     := $(ROOT)/package
INS_PATH    := $(ROOT)/rootfs/rootfs/usr
DLD_PATH    := "-L$(INS_PATH)/lib -L$(CROSS_PATH)/lib "
DCF_PATH    := "-I$(INS_PATH)/include -I$(CROSS_PATH)/include "
DPK_PATH    := $(INS_PATH)/lib/pkgconfig:$(INS_PATH)/share/pkgconfig
KBUDCFLAG   := 
KBLDFLAGS   := 
KCXXFLAGS   := 
KBASFLAGS   := 

# Package information
PACKAGE   := platform_core_module-0.0.1.tar.gz
BASENAME  := platform_core_module-0.0.1
TARCMD    := tar -xvf
PATCH     := patch/$(BASENAME)
URL       := https://gitee.com/BiscuitOS_team/HardStack/raw/Gitee/Device-Driver/Platform/nDTS/Base/kernel
BSFILE    := platform_core_module-0.0.1.bsbit
DLFILE    := platform.c Kconfig Makefile
CONFIG    := --prefix=$(INS_PATH) --host=$(CROSS_NAME)
CONFIG    += --build=
CONFIG    += LDFLAGS="$(KBLDFLAGS)" CFLAGS="$(KBUDCFLAG)"
CONFIG    += CXXFLAGS="$(KCXXFLAGS)" CCASFLAGS="$(KBASFLAGS)"
CONFIG    += LIBS=$(DLD_PATH) CPPFLAGS=$(DCF_PATH)
CONFIG    += PKG_CONFIG_PATH=$(DPK_PATH)
CONFIG    += 


all:
	@cd $(BASENAME) ; \
	PATH=$(CROSS_PATH)/bin:${PATH}  \
	make CROSS_TOOLS=$(CROSS_NAME) \
	BSROOT=$(ROOT) ARCH=$(ARCH) \
	MODULE_NAME=$(BASENAME)
	$(info "Build $(PACKAGE) done.")
	@if [ "${BS_SILENCE}X" != "trueX" ]; then \
		figlet "BiscuitOS" ; \
	fi

prepare:
	@$(BSCORE) bsbit/$(BSFILE) $(PACKDIR)

depence:
	@$(BSDEPD) bsbit/$(BSFILE) $(PACKDIR) $(BASENAME)

depence-clean:
	@rm -rf $(PACKDIR)/.deptmp

download:
	@mkdir -p $(BASENAME)
	@cd $(BASENAME) ; \
	for file in $(DLFILE) ; \
	do \
		LFILE= ; \
		LDIR= ; \
		result=`echo $${file} | grep "/"` ; \
		[ ! -z "$${result}" ] && LFILE=$${file##*/} ; \
		[ ! -z "$${LFILE}" ] && LDIR=$${file%%/$${LFILE}} ; \
		[ ! -z "$${LDIR}" ] && mkdir -p $${LDIR} ; \
		wget $(URL)/$${file} ; \
		[ ! -z "$${LDIR}" ] && mv $${LFILE} $${LDIR} ; \
		echo "Download $${file}" ; \
	done

tar:
	@if [ "${BS_SILENCE}X" != "trueX" ]; then \
		figlet "BiscuitOS" ; \
	fi
	$(info "Decompression $(PACKAGE) => $(BASENAM) done.")

configure:
	@if [ "${BS_SILENCE}X" != "trueX" ]; then \
		figlet "BiscuitOS" ; \
	fi
	$(info "Configure $(BASENAME) done.")

install:
	cd $(BASENAME) ; \
	PATH=$(CROSS_PATH)/bin:${PATH} \
	sudo make install CROSS_TOOLS=$(CROSS_NAME) \
	BSROOT=$(ROOT) ARCH=$(ARCH) PWD=`pwd`
	@if [ "${BS_SILENCE}X" != "trueX" ]; then \
		figlet "BiscuitOS" ; \
	fi
	$(info "Install .... [OK]")

pack:
	@$(PACK) pack
	$(info "Pack    .... [OK]")

build:
	make
	make install pack
	$(ROOT)/RunBiscuitOS.sh

kernel:
	@cd $(ROOT)/linux/linux ; \
	make  ARCH=$(ARCH) bzImage -j4 ;\
	cd - > /dev/null

run:
	$(ROOT)/RunBiscuitOS.sh

clean:
	cd $(BASENAME) ; \
	make clean
	$(info "Clean   .... [OK]")

distclean:
	@rm -rf $(BASENAME)
	$(info "DClean  .... [OK]")


# Reserved by BiscuitOS :)
