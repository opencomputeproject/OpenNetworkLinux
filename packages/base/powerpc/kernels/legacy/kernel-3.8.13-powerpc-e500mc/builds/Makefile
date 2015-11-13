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
COMPDIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

include $(ONL)/make/config.mk

all: kernel-e500mc linux-e500mc-headers dtbs

kernel-e500mc: $(COMPDIR)/kernel-e500mc
linux-e500mc-headers: $(COMPDIR)/linux-e500mc-headers

KERNEL_AS_E500MC := $(COMPDIR)/kernel-e500mc
KERNEL_AS_E500MC_HEADERS := $(COMPDIR)/linux-e500mc-headers
DEFCONFIG := onl_e500mc_defconfig
KERNEL_DIR := $(ONL)/packages/base/any/kernels/legacy/linux-3.8.13

$(COMPDIR)/kernel-e500mc: $(KERNEL_DIR)/arch/powerpc/configs/$(DEFCONFIG)
	mkdir -p $(COMPDIR)/linux-build-e500mc
	$(MAKE) -C $(KERNEL_DIR) $(DEFCONFIG) ARCH=powerpc O=$(COMPDIR)/linux-build-e500mc
	PATH=$$PATH:$(SWITCHLIGHT)/tools $(MAKE) -C $(KERNEL_DIR) uImage ARCH=powerpc O=$(COMPDIR)/linux-build-e500mc
	powerpc-linux-gnu-strip -o $(COMPDIR)/kernel-e500mc $(COMPDIR)/linux-build-e500mc/vmlinux
	cp $(COMPDIR)/linux-build-e500mc/vmlinux.bin.gz $(COMPDIR)/kernel-e500mc.bin.gz

$(COMPDIR)/linux-e500mc-headers/.done: $(KERNEL_DIR)/arch/powerpc/configs/$(DEFCONFIG)
	mkdir -p $(COMPDIR)/linux-e500mc-headers
	cp -a $(KERNEL_DIR)/include $(KERNEL_DIR)/arch/powerpc/include $(COMPDIR)/linux-e500mc-headers
	$(MAKE) -C $(KERNEL_DIR) ARCH=powerpc O=$(COMPDIR)/linux-e500mc-headers $(DEFCONFIG)
	$(MAKE) -C $(KERNEL_DIR) ARCH=powerpc O=$(COMPDIR)/linux-e500mc-headers headers_install modules_prepare arch/powerpc/lib/crtsavres.o
	cp -R $(KERNEL_DIR)/scripts $(COMPDIR)/linux-e500mc-headers
	cp -R $(KERNEL_DIR)/Makefile $(COMPDIR)/linux-e500mc-headers
	cp -R $(KERNEL_DIR)/arch/powerpc/Makefile $(COMPDIR)/linux-e500mc-headers/arch/powerpc
	touch $@

$(COMPDIR)/linux-e500mc-headers: $(COMPDIR)/linux-e500mc-headers/.done


# These are the dtb files built for this kernel configuration:
DTS_LIST := powerpc-accton-as6700-32x-r0 powerpc-accton-as5710-54x-r0 powerpc-as5710-54x-r0a powerpc-as5710-54x-r0b powerpc-accton-as6700-32x-r1 powerpc-accton-as5710-54x-r0b
DTB_LIST := $(foreach dts,$(DTS_LIST),$(COMPDIR)/linux-e500mc-dtbs/$(dts).dtb)

$(COMPDIR)/linux-e500mc-dtbs/%.dtb: $(KERNEL_DIR)/arch/powerpc/boot/dts/%.dts kernel-e500mc | $(shell mkdir -p $(COMPDIR)/linux-e500mc-dtbs)
	linux-build-e500mc/scripts/dtc/dtc -I dts -O dtb -o $@ $<

dtbs: $(DTB_LIST)

.PHONY: deb

deb:
	$(MAKE) -C deb












