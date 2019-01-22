############################################################
#
# Open Network Linux
#
############################################################
include $(ONL)/make/config.mk
export TOOLCHAIN := arm-linux-gnueabihf
export CROSS_COMPILER := $(TOOLCHAIN)-
export ARCH := armhf
export UARCH := ARMHF
export ARCH_BOOT := uboot
export __$(ARCH)__ := 1
