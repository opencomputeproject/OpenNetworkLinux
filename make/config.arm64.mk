############################################################
#
# Open Network Linux
#
############################################################
include $(ONL)/make/config.mk
export TOOLCHAIN := aarch64-linux-gnu
export CROSS_COMPILER := $(TOOLCHAIN)-
export ARCH := arm64
export UARCH := ARM64
export ARCH_BOOT := uboot
export __$(ARCH)__ := 1
