# ncurses
#
# (C) 2019.08.16 BiscuitOS <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

ROOT=/xspace/OpenSource/BiscuitOS/BiscuitOS/output/linux-5.0-arm32
CROSS_NAME=arm-linux-gnueabi
CROSS_PATH=$(ROOT)/$(CROSS_NAME)/$(CROSS_NAME)/bin
CROSS_TOOLS=$(CROSS_PATH)/$(CROSS_NAME)-
PACK=$(ROOT)/RunBiscuitOS.sh
INSTALL_PATH=$(ROOT)/rootfs/rootfs/usr/
PATH += :$(CROSS_PATH)

# Package information
PACKAGE := ncurses-6.1.tar.gz
BASENAM := ncurses-6.1
TARCMD  := tar -xvf
URL     := http://ftp.gnu.org/pub/gnu/ncurses/
CONFIG  := --prefix=$(INSTALL_PATH) --host=$(CROSS_NAME)
CONFIG  += --disable-stripping --with-shared

all:
	cd $(BASENAM) ; \
	make
	$(info "Build .... [OK]")

download:
	wget $(URL)/$(PACKAGE)

tar:
	$(TARCMD) $(PACKAGE)
	$(info "Untar .... [OK]")

configure:
	cd $(BASENAM) ; \
	./configure $(CONFIG) ; \

install:
	cd $(BASENAM) ; \
	make install
	$(info "Install .... [OK]")

pack:
	@$(PACK) pack
	$(info "Pack    .... [OK]")

clean:
	cd $(BASENAM) ; \
	make clean
	$(info "Clean   .... [OK]")

distclean:
	@rm -rf $(BASENAM)
	$(info "DClean  .... [OK]")


# Reserved by BiscuitOS :)
