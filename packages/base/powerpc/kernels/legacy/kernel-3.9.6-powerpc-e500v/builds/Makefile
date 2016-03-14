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
BUILD_POWERPC_LINUX_85XX := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

include $(ONL)/make/config.mk

all: kernel-85xx linux-85xx-headers dtbs

kernel-85xx: $(BUILD_POWERPC_LINUX_85XX)/kernel-85xx
linux-85xx-headers: $(BUILD_POWERPC_LINUX_85XX)/linux-85xx-headers

KERNEL_85XX := $(BUILD_POWERPC_LINUX_85XX)/kernel-85xx
KERNEL_85XX_HEADERS := $(BUILD_POWERPC_LINUX_85XX)/linux-85xx-headers

KERNEL_DIR := $(ONL)/packages/base/any/kernels/legacy/linux-3.9.6

$(BUILD_POWERPC_LINUX_85XX)/kernel-85xx: $(KERNEL_DIR)/arch/powerpc/configs/85xx/onl_mpc85xx_defconfig
	mkdir -p $(BUILD_POWERPC_LINUX_85XX)/linux-build-85xx
	$(MAKE) -C $(KERNEL_DIR) 85xx/onl_mpc85xx_defconfig ARCH=powerpc O=$(BUILD_POWERPC_LINUX_85XX)/linux-build-85xx $(ONL_MAKE_PARALLEL)
	PATH=$$PATH:$(SWITCHLIGHT)/tools $(MAKE) -C $(KERNEL_DIR) uImage ARCH=powerpc O=$(BUILD_POWERPC_LINUX_85XX)/linux-build-85xx
	powerpc-linux-gnu-strip -o $(BUILD_POWERPC_LINUX_85XX)/kernel-85xx $(BUILD_POWERPC_LINUX_85XX)/linux-build-85xx/vmlinux
	cp $(BUILD_POWERPC_LINUX_85XX)/linux-build-85xx/vmlinux.bin.gz $(BUILD_POWERPC_LINUX_85XX)/kernel-85xx.bin.gz

$(BUILD_POWERPC_LINUX_85XX)/linux-85xx-headers/.done: $(KERNEL_DIR)/arch/powerpc/configs/85xx/onl_mpc85xx_defconfig
	mkdir -p $(BUILD_POWERPC_LINUX_85XX)/linux-85xx-headers
	cp -a $(KERNEL_DIR)/include $(KERNEL_DIR)/arch/powerpc/include $(BUILD_POWERPC_LINUX_85XX)/linux-85xx-headers
	$(MAKE) -C $(KERNEL_DIR) ARCH=powerpc O=$(BUILD_POWERPC_LINUX_85XX)/linux-85xx-headers 85xx/onl_mpc85xx_defconfig
	$(MAKE) -C $(KERNEL_DIR) ARCH=powerpc O=$(BUILD_POWERPC_LINUX_85XX)/linux-85xx-headers headers_install modules_prepare arch/powerpc/lib/crtsavres.o
	cp -R $(KERNEL_DIR)/scripts $(BUILD_POWERPC_LINUX_85XX)/linux-85xx-headers
	cp -R $(KERNEL_DIR)/Makefile $(BUILD_POWERPC_LINUX_85XX)/linux-85xx-headers
	cp -R $(KERNEL_DIR)/arch/powerpc/Makefile $(BUILD_POWERPC_LINUX_85XX)/linux-85xx-headers/arch/powerpc
	touch $@

$(BUILD_POWERPC_LINUX_85XX)/linux-85xx-headers: $(BUILD_POWERPC_LINUX_85XX)/linux-85xx-headers/.done

# These are the dtb files built for this kernel configuration:
DTS_LIST := powerpc-quanta-lb9-r0 powerpc-quanta-lb9a-r0 powerpc-quanta-ly2-r0 powerpc-as4600-54t powerpc-as5610-52x powerpc-dni-7448-r0 powerpc-dell-s4810-on-p2020-r0
DTB_LIST := $(foreach dts,$(DTS_LIST),$(BUILD_POWERPC_LINUX_85XX)/linux-85xx-dtbs/$(dts).dtb)

$(BUILD_POWERPC_LINUX_85XX)/linux-85xx-dtbs/%.dtb: $(KERNEL_DIR)/arch/powerpc/boot/dts/%.dts kernel-85xx | $(shell mkdir -p $(BUILD_POWERPC_LINUX_85XX)/linux-85xx-dtbs)
	linux-build-85xx/scripts/dtc/dtc -I dts -O dtb -o $@ $<

dtbs: $(DTB_LIST)


.PHONY: deb

deb:
	$(MAKE) -C deb












