############################################################
#
# Open Network Linux
#
############################################################
include $(ONL)/make/config.mk
export TOOLCHAIN := arm-linux-gnueabi
export CROSS_COMPILER := $(TOOLCHAIN)-
export ARCH := armel
export UARCH := ARMEL
export ARCH_BOOT := uboot
export __$(ARCH)__ := 1
