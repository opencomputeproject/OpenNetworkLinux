# -*- Makefile -*-
############################################################
# <bsn.cl fy=2013 v=none>
#
#        Copyright 2013, 2014 BigSwitch Networks, Inc.
#
#
#
# </bsn.cl>
############################################################
THISDIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
include $(ONL)/make/config.amd64.mk

all: kernel-x86_64 linux-x86_64-headers

LINUX := $(ONL)/packages/base/any/kernels/legacy/linux-3.9.6
ARCH := x86_64

kernel-x86_64: $(LINUX)/arch/x86/configs/onl_x86_64_defconfig
	mkdir -p $(THISDIR)/linux-build-x86_64
	$(MAKE) -C $(LINUX) onl_x86_64_defconfig O=$(THISDIR)/linux-build-x86_64
	$(MAKE) -C $(LINUX) bzImage O=$(THISDIR)/linux-build-x86_64
	cp $(THISDIR)/linux-build-x86_64/arch/x86/boot/bzImage $(THISDIR)/kernel-x86_64


.PHONY: linux-x86_64-headers deb

linux-x86_64-headers: $(LINUX)/arch/x86/configs/onl_x86_64_defconfig
	mkdir -p $(THISDIR)/linux-x86_64-headers
	cp -a $(LINUX)/include $(LINUX)/arch/x86/include $(THISDIR)/linux-x86_64-headers
	$(MAKE) -C $(LINUX) ARCH=x86 O=$(THISDIR)/linux-x86_64-headers onl_x86_64_defconfig
	$(MAKE) -C $(LINUX) ARCH=x86 O=$(THISDIR)/linux-x86_64-headers headers_install modules_prepare

deb:
	$(MAKE) -C deb
